#ifdef CHESS_SERVER_BUILD
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "server.h"
#include "helpers.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace chess_online {
Server::Server(uint16_t port)
    : m_Port(port), m_Address{}, m_DataHandler(nullptr) {
    createSocket();
    m_Address.sin_family = AF_INET;
    m_Address.sin_addr.s_addr = htonl(INADDR_ANY);
    m_Address.sin_port = htons(m_Port);
    bindAndListen();
    setupEpoll();
}

void Server::registerDataHandler(DataHandler handler) {
    m_DataHandler = handler;
}

void Server::registerAcceptHandler(AcceptHandler handler) {
    m_AcceptHandler = handler;
}

void Server::registerDisconnectHandler(DisconnectHandler handler) {
    m_DisconnectHandler = handler;
}

void Server::createSocket() {
    PRINT_MSG("Creating socket..");
    if ((m_SocketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        THROW_RUNTIME_ERROR("Failed to create socket");
    }
    int opt = 1;
    if (setsockopt(m_SocketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        THROW_RUNTIME_ERROR("Failed to set socket options");
    }
}

void Server::bindAndListen() {
    PRINT_MSG("Binding..");
    if (bind(m_SocketFd, (struct sockaddr *)&m_Address, sizeof(m_Address)) < 0) {
        THROW_RUNTIME_ERROR("Failed to bind socket to port: " << m_Port);
    }

    PRINT_MSG("Listening..");
    if (listen(m_SocketFd, BACKLOG_SIZE) < 0) {
        THROW_RUNTIME_ERROR("Failed to listen");
    }
}

void Server::setupEpoll() {
    PRINT_MSG("Setting up Epoll..");
    for (int i = 0; i < NUM_WORKER_THREADS; ++i) {
        if ((m_WorkerEpollFds[i] = epoll_create1(0)) < 0) {
            THROW_RUNTIME_ERROR("Failed to create epoll instance");
        }
    }
}

void Server::addToEpoll(int epfd, int fd) {
    PRINT_MSG("Adding to epoll(epfd, fd): (" << epfd << ", " << fd << ")" << std::endl);

    m_ClientInData.emplace(fd, std::make_shared<Data>());
    m_ClientOutData.emplace(fd, std::make_shared<Data>());

    struct epoll_event event{};
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
        THROW_RUNTIME_ERROR("Failed to add to epoll");
    }
}

void Server::removeFromEpoll(int epfd, int fd) {
    PRINT_MSG("Removing from epoll..");

    m_ClientInData.erase(fd);
    m_ClientOutData.erase(fd);

    m_FdToEpollFd.erase(fd);

    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        THROW_RUNTIME_ERROR("Failed to remove from epoll");
    }
}

void Server::modifyEpoll(int epfd, int fd, uint32_t events) {
    // PRINT_MSG("Modifying epoll for fd: " << fd << " with events: " << events);

    struct epoll_event event{};
    event.events = events | EPOLLET; // Always use edge-triggered mode
    event.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0) {
        THROW_RUNTIME_ERROR("Failed to modify epoll for fd " << fd << ": " << errno);
    }
}

void Server::listenThread() {
    PRINT_MSG("Listen thread running..");
    sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientSocket;
    std::array<char, 16> clientIp{};
    while (1) {
        if ((clientSocket = accept4(m_SocketFd, (sockaddr *)&clientAddress, &clientLen, SOCK_NONBLOCK)) < 0) {
            continue;
        }

        inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp.data(), INET_ADDRSTRLEN);

        PRINT_MSG("Accepted connection from: " << clientIp.data() << " with socket value: " << clientSocket);

        addToEpoll(m_WorkerEpollFds[m_CurrentWorker], clientSocket);
        m_FdToEpollFd.emplace(clientSocket, m_WorkerEpollFds[m_CurrentWorker]);
        m_CurrentWorker++;
        if (m_CurrentWorker >= NUM_WORKER_THREADS) {
            m_CurrentWorker = 0;
        }

        if (m_AcceptHandler) {
            m_AcceptHandler(clientSocket);
        }
    }
}

void Server::handleThread(int workerId) {
    std::ostringstream msg;
    PRINT_MSG("Starting worker thread: " << std::this_thread::get_id() << " with workerId: " << workerId);
    while (1) {
        int eventsReady = epoll_wait(m_WorkerEpollFds[workerId], m_WorkerEpollEvents[workerId], NUM_EPOLL_EVENTS_MAX, 0);
        if (eventsReady <= 0) {
            continue;
        }
        // PRINT_MSG("Events ready: " << eventsReady);
        for (int i = 0; i < eventsReady; ++i) {
            epoll_event currentEvent = m_WorkerEpollEvents[workerId][i];
            int currentClientFd = currentEvent.data.fd;
            // PRINT_MSG("WorkerId: " << workerId);
            // PRINT_MSG("Events: " << currentEvent.events);
            if (currentClientFd == m_SocketFd) {
                PRINT_MSG("It is the server fd, event: " << currentEvent.events);
                continue;
            }
            if (currentEvent.events & (EPOLLHUP | EPOLLERR)) {
                closeConnection(m_WorkerEpollFds[workerId], currentClientFd);
                continue;
            }
            if (currentEvent.events & EPOLLOUT) {
                if (handleWrite(currentClientFd) < 0) {
                    closeConnection(m_WorkerEpollFds[workerId], currentClientFd);
                    continue;
                }
                modifyEpoll(m_WorkerEpollFds[workerId], currentClientFd, EPOLLIN);
            }
            if (currentEvent.events & EPOLLIN) {
                if (handleRead(currentClientFd) <= 0) {
                    closeConnection(m_WorkerEpollFds[workerId], currentClientFd);
                    continue;
                }
                modifyEpoll(m_WorkerEpollFds[workerId], currentClientFd, EPOLLOUT);
            }
        }
    }
}

int Server::handleRead(int clientFd) {
    int bytesReceived = recv(clientFd, m_ClientInData[clientFd]->buffer.data(), MAX_BUFFER_SIZE, 0);
    m_ClientInData[clientFd]->len = bytesReceived;
    m_ClientInData[clientFd]->pos = 0;
    PRINT_MSG("Received number of bytes = " << bytesReceived << std::endl);

    if (m_DataHandler) {
        m_DataHandler(clientFd, *m_ClientInData[clientFd], *m_ClientOutData[clientFd]);
    }

    return bytesReceived;
}

// Sends outs everything that is in the out buffer
int Server::handleWrite(int clientFd) {
    PRINT_MSG("handleWrite called");
    Data outData = *m_ClientOutData[clientFd];
    int totalSent = 0;
    while (outData.pos < outData.len) {
        int bytesSent = send(clientFd, m_ClientOutData[clientFd]->buffer.data(), outData.len - outData.pos, 0);
        PRINT_MSG(clientFd);
        if (bytesSent <= 0) {
            // Retry sending if it would block
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return bytesSent;
        }
        outData.pos += bytesSent;
        totalSent += bytesSent;
    }
    return totalSent;
}

int Server::sendMessage(int recipientFd, const std::vector<char> &data) {
    PRINT_MSG("Send message called: " << std::endl);
    std::copy(data.begin(), data.end(), m_ClientOutData[recipientFd]->buffer.begin());
    m_ClientOutData[recipientFd]->len = data.size();
    m_ClientOutData[recipientFd]->pos = 0;
    modifyEpoll(m_FdToEpollFd[recipientFd], recipientFd, EPOLLOUT);
    return 0;
}

void Server::closeConnection(int epfd, int fd) {
    PRINT_MSG("Closing connection on epfd,fd: " << epfd << "," << fd);
    removeFromEpoll(epfd, fd);
    close(fd);
    if (m_DisconnectHandler) {
        m_DisconnectHandler(fd);
    }
}

void Server::run() {
    PRINT_MSG("Run called..");
    m_ListeningThread = std::thread(&Server::listenThread, this);
    for (int i = 0; i < NUM_WORKER_THREADS; ++i) {
        m_WorkerThreads[i] = std::thread(&Server::handleThread, this, i);
    }
    while (1) {
        sleep(2147483647);
    }
}
} // namespace chess_online
#endif