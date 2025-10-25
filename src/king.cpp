#include "king.h"

namespace chess_client {
King::King(int x, int y, bool isBlack)
    : Piece(x, y, isBlack) {
    loadSurface(isBlack ? "res/b_king.png" : "res/w_king.png");
};
std::vector<Position> King::getPossibleMoves(std::array<Square, 64>& board) {
    std::vector<Position> moves;
    // King movement logic to be implemented
    return moves;
};
} // namespace chess_client