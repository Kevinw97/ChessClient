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
    RenderHandler m_RenderHandler;
    std::array<Square, 64> m_Board;
    std::set<std::shared_ptr<Piece>> m_WhitePieces;
    std::set<std::shared_ptr<Piece>> m_BlackPieces;
    std::shared_ptr<Piece> m_BlackKing = nullptr;
    std::shared_ptr<Piece> m_WhiteKing = nullptr;
    PlayerColor m_CurrentPlayerColor = WHITE;
    PlayerColor m_CurrentTurnColor = WHITE;
    Square* m_SelectedSquare = nullptr;

    void setupInitialPieces(std::array<Square, 64> &board);
    void gameLoop();
    void handleMouseClick(SDL_Event* event);
    Square *getSquareAtPosition(std::array<Square, 64> &board, int x, int y);
    bool isCurrentPlayersTurn();
    bool isValidMove(const Move &move);
    bool isKingInCheck(std::array<Square, 64> &board, PlayerColor Color);
    void processMove(int srcX, int srcY, int dstX, int dstY);
    void selectSource(int x, int y);
    void selectDestination(int x, int y);
    void unselectAllSquares();

public:
    ChessGame();
    void run();
};

} // namespace chess_client