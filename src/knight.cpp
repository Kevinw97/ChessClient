#include "knight.h"

namespace chess_client
{
  Knight::Knight(int x, int y, bool isBlack)
    : Piece(x, y, isBlack)
  {
    loadSurface(isBlack ? "res/b_knight.png" : "res/w_knight.png");
  };
  std::vector<Position> Knight::getPossibleMoves()
  {
    std::vector<Position> moves;
    // Knight movement logic to be implemented
    return moves;
  };
}  // namespace chess_client