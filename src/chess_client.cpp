#ifdef CHESS_CLIENT_BUILD
#include "chess_client.h"

namespace chess_online {
ChessClient::ChessClient()
    : m_WsaData{}, m_ClientSocket(INVALID_SOCKET) {
    m_InBuffer.reserve(4096);
    m_OutBuffer.reserve(4096);
}

ChessClient::~ChessClient() {
    cleanWsa();
    if (m_ListenerThread.joinable()) {
        m_ListenerThread.join();
    }
}

void ChessClient::initClient(GameHandler handler) {
    m_GameHandler = handler;
    std::string userInput;
    std::cout << "Enter server address: ";
    while (std::getline(std::cin, userInput)) {
        struct sockaddr_in sa;
        int result = inet_pton(AF_INET, userInput.c_str(), &(sa.sin_addr));
        if (result) {
            m_ServerIp = userInput;
            break;
        } else {
            std::cout << "Invalid input. Enter a valid ip address: ";
        }
    }

    int result = WSAStartup(MAKEWORD(2, 2), &m_WsaData);
    if (result != 0) {
        LOG_COUT("WSAStartup failed: " << result);
        ;
        return;
    }
    m_WsaStarted = true;

    m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_ClientSocket == INVALID_SOCKET) {
        LOG_COUT("Socket creation failed: " << WSAGetLastError());
        WSACleanup();
        return;
    }

    // Setup server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12312);
    inet_pton(AF_INET, m_ServerIp.c_str(), &serverAddr.sin_addr);

    // Connect to server
    result = connect(m_ClientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        LOG_COUT("Connection failed: " << WSAGetLastError());
        closesocket(m_ClientSocket);
        WSACleanup();
        return;
    }

    std::cout << "Connected to server!" << std::endl;

    std::cout << "Waiting for an opponent.." << std::endl;
    // Receive match setup information from server
    char buffer[256] = {0};
    int bytesReceived = recv(m_ClientSocket, buffer, 256, 0);
    if (bytesReceived <= 0) {
        LOG_COUT("Connection closed by server or recv failed");
        closesocket(m_ClientSocket);
        WSACleanup();
    } else {
        m_AssignedColor = static_cast<PieceColor>(buffer[0]);
    }

    m_ListenerThread = std::thread(&ChessClient::listenLoop, this);
}

void ChessClient::listenLoop() {
    while (true) {
        int bytesReceived = recv(m_ClientSocket, m_InBuffer.data(), 4096, 0);
        if (bytesReceived <= 0) {
            std::cout << "Connection closed by server, or recv failed" << std::endl;
            break;
        }

        // Server will send the 0x55 command, piece key, NetworkMove, serializedBoard, and the turn color
        if (bytesReceived >= 2 + sizeof(NetworkMove) + NUM_SQUARES + 1) {
            std::cout << "Received " << bytesReceived << " bytes:" << std::endl;
            int i = 0;
            unsigned char *data = reinterpret_cast<unsigned char *>(m_InBuffer.data());
            if (data[i] == 0x55) {
                i++;

                // Get piece
                if (i + 1 > bytesReceived) {
                    std::cerr << "Didn't receive the piece\n";
                    throw std::runtime_error("Didn't receive the piece\n");
                }
                unsigned char pieceKey = data[i];
                i++;

                // Get network move
                const NetworkMove *networkMovePtr = reinterpret_cast<const NetworkMove *>(&data[i]);
                NetworkMove networkMove = *networkMovePtr;
                i += sizeof(NetworkMove);

                std::array<unsigned char, NUM_SQUARES> serializedBoard;
                std::copy(&data[i], &data[i] + NUM_SQUARES, serializedBoard.begin());
                i += NUM_SQUARES;

                PieceColor turnColor = static_cast<PieceColor>(data[i]);

                // Process the move, and validate that the board matches the state sent by server
                m_GameHandler(pieceKey, networkMove, serializedBoard, turnColor);
            }
        }
    }
}

void ChessClient::writeMove(const std::shared_ptr<Piece> &piece, const Move &move) {
    NetworkMove networkMove;
    networkMove.src = move.src;
    networkMove.dst = move.dst;
    networkMove.capturedPiece = move.capturedPiece ? move.capturedPiece->getPieceKey() : 0;
    networkMove.castlingRookSrc = move.castlingRookSrc;
    networkMove.castlingRookDst = move.castlingRookDst;
    networkMove.castlingRook = move.castlingRook ? move.castlingRook->getPieceKey() : 0;
    networkMove.promoteType = move.promoteType;
    networkMove.firstMove = move.firstMove;

    const unsigned char *moveBytes = reinterpret_cast<const unsigned char *>(&networkMove);
    m_OutBuffer.push_back(piece->getPieceKey());
    m_OutBuffer.insert(m_OutBuffer.end(), moveBytes, moveBytes + sizeof(NetworkMove));
}

void ChessClient::sendMove(
    std::array<unsigned char, NUM_SQUARES> board,
    const std::shared_ptr<Piece> &piece,
    const Move &move) {
    // Send move to server and validate
    m_OutBuffer.clear();
    m_OutBuffer.push_back(0x55);
    m_OutBuffer.insert(m_OutBuffer.end(), board.begin(), board.end());
    writeMove(piece, move);
    send(m_ClientSocket, reinterpret_cast<const char *>(m_OutBuffer.data()), static_cast<int>(m_OutBuffer.size()), 0);
    m_OutBuffer.clear();
}

void ChessClient::cleanWsa() {
    if (m_ClientSocket != INVALID_SOCKET) {
        closesocket(m_ClientSocket);
    }
    if (m_WsaStarted) {
        WSACleanup();
    }
}
} // namespace chess_online
#endif