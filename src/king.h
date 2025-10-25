#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class King : public Piece {
public:
  King(int x, int y, bool isBlack);
  std::vector<Position> getPossibleMoves(std::array<Square, 64>& board) override;
};
} // namespace chess_client