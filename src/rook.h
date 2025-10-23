#pragma once
#include "piece.h"

namespace chess_client
{
  class Rook : public Piece
  {
  public:
    Rook(int x, int y, bool isBlack);
    std::vector<Position> getPossibleMoves() override;
  };
}  // namespace chess_client