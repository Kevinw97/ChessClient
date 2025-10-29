#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class Bishop : public Piece {
public:
  Bishop(Square *square, PieceColor color);
  std::vector<Move> getPossibleMoves(const std::array<Square, 64> &board, const std::vector<Action> &actionHistory) override;
  PieceType getType() override { return BISHOP; };
};
} // namespace chess_client