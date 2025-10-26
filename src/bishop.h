#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class Bishop : public Piece {
public:
  Bishop(Square *square, bool isBlack);
  std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board) override;
};
} // namespace chess_client