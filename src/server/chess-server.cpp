#ifdef CHESS_SERVER_BUILD
#include "chess-server.h"
#include "helpers.h"
#include "server.h"

namespace chess_online {
ChessServer::ChessServer() : m_Server(Server(12312)) {
    m_Server.registerDataHandler([this](int client, Data &inData, Data &outData) {
        responseHandler(client, inData, outData);
    });
    m_Server.registerAcceptHandler([this](int client) {
        acceptHandler(client);
    });
    m_Server.registerDisconnectHandler([this](int client) {
        disconnectHandler(client);
    });
}

void ChessServer::responseHandler(int client, Data &inData, Data &outData) {
    std::shared_ptr<ChessGame> game = m_ClientGames[client];
    if (!game) {
        PRINT_MSG("No game exists");
        return;
    }
    if (inData.len < sizeof(unsigned char) + NUM_SQUARES + sizeof(unsigned char) + sizeof(NetworkMove)) {
        PRINT_MSG("Did not receive entire command");
    }
    std::mutex &gameMutex = game->getMutex();
    if (gameMutex.try_lock()) {
        PRINT_MSG("In the lock block");
        int i = 0;
        std::array<unsigned char, MAX_BUFFER_SIZE> response;
        std::copy(inData.buffer.begin(), inData.buffer.begin() + inData.len, response.begin());
        if (response[i] == 0x55) {
            PRINT_MSG("Received proper command");
            i++;

            // Check if we got the entire board, validate entire board
            std::array<unsigned char, NUM_SQUARES> boardData;
            std::copy(response.begin() + i, response.begin() + i + NUM_SQUARES, boardData.begin());
            if (!game->validateBoard(boardData)) {
                PRINT_MSG("Board received was invalid");
                gameMutex.unlock();
                return;
            }
            i += NUM_SQUARES;

            // Check for the piece
            unsigned char pieceKey = response[i];
            std::shared_ptr<Piece> piece = game->getPiece(static_cast<unsigned char>(response[i]));
            if (!piece) {
                PRINT_MSG("Couldn't get piece");
                gameMutex.unlock();
                return;
            }
            i++;

            // Check for the move
            NetworkMove networkMove;
            std::memcpy(&networkMove, &response[i], sizeof(NetworkMove));
            Move move = game->decodeMove(networkMove);

            // Process the game
            game->processMove(piece, move);

            PRINT_MSG("Processed the game!");

            std::array<char, 2 + sizeof(NetworkMove) + 64 + 1> message;
            message[0] = 0x55;
            message[1] = pieceKey;
            std::memcpy(&message[2], &networkMove, sizeof(NetworkMove));
            std::memcpy(&message[2 + sizeof(NetworkMove)], game->serializeBoard().data(), 64);
            message[2 + sizeof(NetworkMove) + 64] = game->getTurn();

            // Copy data to out buffer of client, which server will send out
            PRINT_MSG("Copying to out buffer of client");
            std::copy(message.begin(), message.end(), outData.buffer.begin());
            outData.len = message.size();
            outData.pos = 0;

            // Relay message to opponent
            int opponent = m_ClientPairings[client];
            PRINT_MSG("Sending to opponent");
            if (m_ConnectedClients.find(opponent) != m_ConnectedClients.end()) {
                m_Server.sendMessage(opponent, std::vector(message.begin(), message.end()));
            }

            if (game->isCheckmate()) {
                eraseClientAndOpponent(client);
            }

            gameMutex.unlock();
            return;
        }
    } else {
        return;
    }
}

void ChessServer::acceptHandler(int client) {
    PRINT_MSG("Connection received in accept handler from: " << client);
    m_ConnectedClients.insert(client);

    if (m_ClientsWaitingForMatch.empty()) {
        // Just adding this opponent to the waiting list, no need to lock
        m_ClientsWaitingForMatch.insert(client);
    } else {
        std::scoped_lock lock(m_MatchingMutex);
        // Need to lock as we read and match the opponent and pop it from the list
        int opponent = *m_ClientsWaitingForMatch.rbegin();

        m_ClientsWaitingForMatch.erase(opponent);

        m_ClientPairings.emplace(client, opponent);
        m_ClientPairings.emplace(opponent, client);

        std::shared_ptr<ChessGame> newGame = std::make_shared<ChessGame>();

        m_ClientGames.emplace(client, newGame);
        m_ClientGames.emplace(opponent, newGame);

        std::vector<char> opponentMessage;
        opponentMessage.push_back(WHITE);
        m_Server.sendMessage(opponent, opponentMessage);

        std::vector<char> clientMessage;
        clientMessage.push_back(BLACK);
        m_Server.sendMessage(client, clientMessage);
    }
}

void ChessServer::disconnectHandler(int client) {
    PRINT_MSG("Disconnected!");
    eraseClientAndOpponent(client);
}

void ChessServer::eraseClientAndOpponent(int client) {
    if (m_EraseMutex.try_lock()) {
        // This client has not been matched yet
        if (m_ClientPairings.find(client) == m_ClientPairings.end()) {
            if (m_ConnectedClients.find(client) != m_ConnectedClients.end()) {
                PRINT_MSG("Erasing client from connected clients!");
                m_ConnectedClients.erase(client);
            }
            if (m_ClientsWaitingForMatch.find(client) != m_ClientsWaitingForMatch.end()) {
                PRINT_MSG("Erasing client from waiting from matches!");
                m_ClientsWaitingForMatch.erase(client);
            }
        } else {
            // Erasing a client that has been amtched
            int opponent = m_ClientPairings[client];
            if (m_ConnectedClients.find(client) != m_ConnectedClients.end()) {
                PRINT_MSG("Erasing client from connected clients!");
                m_ConnectedClients.erase(client);
            }
            if (m_ConnectedClients.find(opponent) != m_ConnectedClients.end()) {
                PRINT_MSG("Erasing opponent from connected clients!");
                m_ConnectedClients.erase(opponent);
            }
            if (m_ClientPairings.count(client)) {
                PRINT_MSG("Erasing pairings: " << client << " " << opponent);
                m_ClientPairings.erase(client);
                m_ClientGames.erase(client);
            }
            if (m_ClientPairings.count(opponent)) {
                PRINT_MSG("Erasing pairings: " << opponent << " " << client);
                m_ClientPairings.erase(opponent);
                m_ClientGames.erase(opponent);
            }
        }
        m_EraseMutex.unlock();
    }
}

void ChessServer::run() {
    m_Server.run();
}
} // namespace chess_online
#endif