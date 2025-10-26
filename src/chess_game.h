#pragma once
#include "bishop.h"
#include "king.h"
#include "knight.h"
#include "pawn.h"
#include "piece.h"
#include "queen.h"
#include "rook.h"
#include "render_handler.h"
#include "chess.h"
#include <set>

namespace chess_client {
  class ChessGame {

  private:
    bool m_InProgress = true;
    bool m_Running = true;
    RenderHandler m_RenderHandler;
    std::array<Square, 64> m_Board;
    std::set<std::shared_ptr<Piece>> m_WhitePieces;
    std::set<std::shared_ptr<Piece>> m_BlackPieces;
    std::shared_ptr<Piece> m_BlackKing = nullptr;
    std::shared_ptr<Piece> m_WhiteKing = nullptr;
    PieceColor m_PlayerColor = WHITE;
    PieceColor m_CurrentTurnColor = WHITE;
    Square* m_SelectedSquare = nullptr;
    std::vector<Move> m_MovesForSelected;
    std::vector<Action> m_ActionHistory;

    void setupInitialPieces(std::array<Square, 64>& board);
    void gameLoop();
    void handleMouseClick(SDL_Event* event);
    bool isCurrentPlayersTurn();
    bool isValidMove(const std::shared_ptr<Piece>& piece, const Move& move);
    bool isKingInCheck(std::array<Square, 64>& board, PieceColor Color);
    void processMove(const std::shared_ptr<Piece>& piece, Move& move, bool undo = false);
    void selectSource(int x, int y);
    void selectDestination(int x, int y);
    void unselectAllSquares();
    void undoMove();
    void resetGame();

  public:
    ChessGame();
    void run();
  };

} // namespace chess_client