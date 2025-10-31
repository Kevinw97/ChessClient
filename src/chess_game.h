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
#include "chess_client.h"

namespace chess_client {
  class ChessGame {
  private:
    bool m_InProgress = true;
    bool m_Running = true;
    std::array<Square, NUM_SQUARES> m_Board;
    RenderHandler m_RenderHandler;
    AudioHandler m_AudioHandler;
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

    bool m_IsOnline = false;
    ChessClient m_ChessClient;

    void setupInitialPieces(std::array<Square, NUM_SQUARES>& board);
    void gameSetup();
    void gameLoop();
    void listenLoop();
    void handleMouseClick(SDL_Event* event);
    bool isCurrentPlayersTurn();
    bool isValidMove(const std::shared_ptr<Piece>& piece, const Move& move);
    bool isKingInCheck(std::array<Square, NUM_SQUARES>& board, PieceColor Color);
    void processMove(const std::shared_ptr<Piece>& piece, const Move& move);
    void clientMoveHandler(
      unsigned char pieceKey,
      NetworkMove networkMove,
      std::array<unsigned char, NUM_SQUARES> serializedBoard,
      PieceColor turnColor);
    void selectSource(int x, int y);
    void selectDestination(int x, int y);
    void unselectAllSquares();
    void undoMove();
    Move decodeMove(NetworkMove data);
    void resetGame();
    std::shared_ptr<Piece> getPiece(unsigned char pieceKey);
    bool validateBoard(std::array<unsigned char, NUM_SQUARES> board);
    void choosePawnPromotion(const std::shared_ptr<Piece>& piece, Move& move);
    std::array<unsigned char, NUM_SQUARES> serializeBoard();

  public:
    ChessGame();
    void run();

    friend class ChessClient;
  };

} // namespace chess_client