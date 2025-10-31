#ifdef CHESS_SERVER_BUILD
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <set>
#include <string>
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#define NUM_WORKER_THREADS 8
#define NUM_EPOLL_EVENTS_MAX 1000
#define BACKLOG_SIZE 1000
#define MAX_BUFFER_SIZE 4096U

namespace chess_online {

struct Data {
    Data() : len(0), pos(0), buffer() {};
    size_t len;
    size_t pos;
    std::array<char, MAX_BUFFER_SIZE> buffer;
};

using DataHandler = std::function<void(int clientFd, Data &inData, Data &outData)>;
using AcceptHandler = std::function<void(int clientFd)>;
using DisconnectHandler = std::function<void(int clientFd)>;

class Server {
public:
    explicit Server(uint16_t port); // Create socket, bind, setup epoll
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) noexcept = default;
    Server &operator=(Server &&) noexcept = default;
    void registerDataHandler(DataHandler handler);
    void registerAcceptHandler(AcceptHandler handler);
    void registerDisconnectHandler(DisconnectHandler handler);
    void run(); // Start listening, setup listening and worker threads
    int sendMessage(int recipientFd, const std::vector<char> &data);

private:
    uint16_t m_Port;
    int m_SocketFd;
    struct sockaddr_in m_Address;
    std::thread m_ListeningThread;
    std::thread m_WorkerThreads[NUM_WORKER_THREADS];
    int m_WorkerEpollFds[NUM_WORKER_THREADS];
    epoll_event m_WorkerEpollEvents[NUM_WORKER_THREADS][NUM_EPOLL_EVENTS_MAX];
    std::unordered_map<int, std::shared_ptr<Data>> m_ClientInData;
    std::unordered_map<int, std::shared_ptr<Data>> m_ClientOutData;
    std::unordered_map<int, int> m_FdToEpollFd;
    AcceptHandler m_AcceptHandler;
    DataHandler m_DataHandler;
    DisconnectHandler m_DisconnectHandler;

    int m_CurrentWorker = 0;

    void createSocket();
    void bindAndListen();
    void setupEpoll();
    void addToEpoll(int epfd, int fd);
    void removeFromEpoll(int epfd, int fd);
    void modifyEpoll(int epfd, int fd, uint32_t events);
    void listenThread();
    void handleThread(int workerId);
    void closeConnection(int workerId, int fd);
    int handleRead(int clientFd);
    int handleWrite(int clientFd);
};
}; // namespace chess_online

#endif