#pragma once
#include "piece.h"

namespace chess_client
{
  class Pawn : public Piece
  {
  public:
    Pawn(int x, int y, bool isBlack);
    std::vector<Position> getPossibleMoves() override;
  };
}  // namespace chess_client