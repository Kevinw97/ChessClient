#pragma once
#include "piece.h"

namespace chess_client
{
  class Bishop : public Piece
  {
  public:
    Bishop(int x, int y, bool isBlack);
    std::vector<Position> getPossibleMoves() override;
  };
}  // namespace chess_client