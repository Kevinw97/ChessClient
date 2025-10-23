#include "queen.h"

namespace chess_client
{
  Queen::Queen(int x, int y, bool isBlack)
    : Piece(x, y, isBlack)
  {
    loadSurface(isBlack ? "res/b_queen.png" : "res/w_queen.png");
  };
  std::vector<Position> Queen::getPossibleMoves()
  {
    std::vector<Position> moves;
    // Queen movement logic to be implemented
    return moves;
  };
}  // namespace chess_client