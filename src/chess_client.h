#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  using GameHandler = std::function<void(
    unsigned char, NetworkMove, std::array<unsigned char, NUM_SQUARES>, PieceColor)>;

  class ChessClient {
  private:
    WSADATA m_WsaData;
    bool m_WsaStarted = false;
    SOCKET m_ClientSocket;
    std::string m_ServerIp;
    std::thread m_ListenerThread;
    std::vector<char> m_OutBuffer;
    std::vector<char> m_InBuffer;
    PieceColor m_AssignedColor = WHITE;
    GameHandler m_GameHandler;

    void listenLoop();
    void cleanWsa();

  public:
    ChessClient();
    ~ChessClient();
    void initClient(GameHandler handler);
    void writeMove(const std::shared_ptr<Piece>& piece, const Move& move);
    void sendMove(std::array<unsigned char, NUM_SQUARES> board, const std::shared_ptr<Piece>& piece, const Move& move);
    PieceColor getAssignedColor() { return m_AssignedColor; };
  };
}