#ifdef CHESS_SERVER_BUILD
#include "../chess.h"
#include "../chess_game.h"
#include "server.h"

namespace chess_online {

enum Command : unsigned char {
    MOVE
};

struct ClientInfo {
    PieceColor color;
};

class ChessServer {
public:
    ChessServer();
    ChessServer(const ChessServer &) = delete;
    ChessServer &operator=(const ChessServer &) = delete;
    ChessServer(ChessServer &&) noexcept = default;
    ChessServer &operator=(ChessServer &&) noexcept = default;

    void run();

private:
    Server m_Server;
    std::unordered_set<int> m_ConnectedClients;                        // List of all clients still connected
    std::unordered_map<int, int> m_ClientPairings;                     // Map from one clientFd to another. For every pair (X, Y), there will be two mappings from X->Y and Y->X
    std::unordered_map<int, std::shared_ptr<ChessGame>> m_ClientGames; // All ongoing games
    std::set<int> m_ClientsWaitingForMatch;                            // Clients waiting for a match
    std::mutex m_MatchingMutex;                                        // When matching a player to an opponent, we need a mutex to make sure it doesn't match the same opponent with someone waiting
    std::mutex m_EraseMutex;                                           // Once one player disconnects, the player and opponent are kicked off, make sure both don't disconnect at same time

    void responseHandler(int client, Data &inData, Data &outData);
    void acceptHandler(int client);
    void disconnectHandler(int client);
    void eraseClientAndOpponent(int client);
};
} // namespace chess_online
#endif