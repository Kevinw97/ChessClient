#include "pawn.h"
#include "chess.h"

namespace chess_client {
Pawn::Pawn(int x, int y, bool isBlack) : Piece(x, y, isBlack) {
  loadSurface(isBlack ? "res/b_pawn.png" : "res/w_pawn.png");
};
std::vector<Position> Pawn::getPossibleMoves(std::array<Square, 64> board) {
  std::vector<Position> moves;
  if (isBlack()) {
    // Black pawn moves down the board
    moves.push_back({getPosition().x, getPosition().y + 1});
    if (getPosition().y == 1) {
      // Initial double move
      moves.push_back({getPosition().x, getPosition().y + 2});
    }
  } else {
    // White pawn moves up the board
    moves.push_back({getPosition().x, getPosition().y - 1});
    if (getPosition().y == 6) {
      // Initial double move
      moves.push_back({getPosition().x, getPosition().y - 2});
    }
  }
  return moves;
};
} // namespace chess_client