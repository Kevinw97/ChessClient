#pragma once
#include "bishop.h"
#include "king.h"
#include "knight.h"
#include "pawn.h"
#include "piece.h"
#include "queen.h"
#include "rook.h"
#include "sdl_handler.h"
#include <set>
#include "chess.h"

namespace chess_client {
class ChessGame {

private:
    RenderHandler m_RenderHandler;
    std::set<std::shared_ptr<Piece>> m_WhitePieces;
    std::set<std::shared_ptr<Piece>> m_BlackPieces;
    bool m_PlayerIsBlack = false;
    bool m_IsBlackTurn = false;
    Square* m_SelectedSquare = nullptr;
    std::array<Square, 64> m_Board;

    void setupInitialPieces();
    void gameLoop();
    void handleMouseClick(SDL_Event* event);
    Square* getSquareAtPosition(int x, int y);
    bool isCurrentPlayersTurn();
    void processMove(int srcX, int srcY, int dstX, int dstY);
    void selectSource(int x, int y);
    void selectDestination(int x, int y);
    void unselectAllSquares();

public:
    ChessGame();
    void run();
};

} // namespace chess_client