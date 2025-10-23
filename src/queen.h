#pragma once
#include "piece.h"

namespace chess_client
{
  class Queen : public Piece
  {
  public:
    Queen(int x, int y, bool isBlack);
    std::vector<Position> getPossibleMoves() override;
  };
}  // namespace chess_client
