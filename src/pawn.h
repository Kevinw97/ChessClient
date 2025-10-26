#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class Pawn : public Piece {
public:
  Pawn(Square *square, bool isBlack);
  std::vector<Move> getPossibleMoves(const std::array<Square, 64> &board) override;
};
} // namespace chess_client