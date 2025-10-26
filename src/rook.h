#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class Rook : public Piece {
public:
  Rook(Square *square, bool isBlack);
  std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board) override;
};
} // namespace chess_client