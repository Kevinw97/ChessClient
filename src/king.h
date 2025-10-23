#pragma once
#include "piece.h"

namespace chess_client
{
  class King : public Piece
  {
  public:
    King(int x, int y, bool isBlack);
    std::vector<Position> getPossibleMoves() override;
  };
}