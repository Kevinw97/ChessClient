#pragma once
#include "bishop.h"
#include "king.h"
#include "knight.h"
#include "pawn.h"
#include "piece.h"
#include "queen.h"
#include "rook.h"
#include "sdl_render_handler.h"
#include "sdl_audio_handler.h"
#include "chess.h"

namespace chess_client {
  class ChessGame {

  private:
    bool m_InProgress = true;
    bool m_Running = true;
    bool m_Online = false;
    GameStatus m_State;
    std::thread m_ListenerThread;
    std::string m_ServerIp;
    SOCKET m_ClientSocket;
    std::vector<char> m_DataBuffer;
    RenderHandler m_RenderHandler;
    AudioHandler m_AudioHandler;
    std::array<Square, 64> m_Board;
    std::set<std::shared_ptr<Piece>> m_WhitePieces;
    std::set<std::shared_ptr<Piece>> m_BlackPieces;
    std::unordered_map<unsigned char, std::shared_ptr<Piece>> m_Pieces;
    std::shared_ptr<Piece> m_BlackKing = nullptr;
    std::shared_ptr<Piece> m_WhiteKing = nullptr;
    PieceColor m_PlayerColor = WHITE;
    PieceColor m_CurrentTurnColor = WHITE;
    Square* m_SelectedSquare = nullptr;
    std::vector<Move> m_MovesForSelected;
    std::vector<Action> m_ActionHistory;

    void setupInitialPieces(std::array<Square, 64>& board);
    void gameSetup();
    void gameLoop();
    void listenLoop();
    void setupOnlineClient();
    void handleMouseClick(SDL_Event* event);
    bool isCurrentPlayersTurn();
    bool isValidMove(const std::shared_ptr<Piece>& piece, const Move& move);
    bool isKingInCheck(std::array<Square, 64>& board, PieceColor Color);
    void processMove(const std::shared_ptr<Piece>& piece, const Move& move, bool clientProcess);
    void selectSource(int x, int y);
    void selectDestination(int x, int y);
    void unselectAllSquares();
    void undoMove();
    void writeBoard();
    void writeMove(const std::shared_ptr<Piece>& piece, const Move& move);
    void sendCommand();
    Move decodeMove(unsigned char* data);
    void resetGame();
    unsigned char getPieceKey(const std::shared_ptr<Piece>& piece);
    std::shared_ptr<Piece> getPiece(unsigned char pieceKey);
    bool validateBoard(unsigned char* board);

  public:
    ChessGame();
    void run();
  };

} // namespace chess_client