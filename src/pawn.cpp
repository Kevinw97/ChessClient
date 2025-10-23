#include "pawn.h"

namespace chess_client
{
  Pawn::Pawn(int x, int y, bool isBlack)
    : Piece(x, y, isBlack)
  {
    loadSurface(isBlack ? "res/b_pawn.png" : "res/w_pawn.png");
  };
  std::vector<Position> Pawn::getPossibleMoves()
  {
    std::vector<Position> moves;
    // Pawn movement logic to be implemented
    return moves;
  };
}  // namespace chess_client