#include "rook.h"

namespace chess_client
{
  Rook::Rook(int x, int y, bool isBlack)
    : Piece(x, y, isBlack)
  {
    loadSurface(isBlack ? "res/b_rook.png" : "res/w_rook.png");
  };
  std::vector<Position> Rook::getPossibleMoves()
  {
    std::vector<Position> moves;
    // Rook movement logic to be implemented
    return moves;
  };
}  // namespace chess_client