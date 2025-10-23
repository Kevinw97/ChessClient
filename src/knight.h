#pragma once
#include "piece.h"

namespace chess_client
{
  class Knight : public Piece
  {
  public:
    Knight(int x, int y, bool isBlack);
    std::vector<Position> getPossibleMoves() override;
  };
}  // namespace chess_client
