#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  class Knight : public Piece {
  public:
    Knight(Square* square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, NUM_SQUARES>& board, const std::vector<Action>& actionHistory) override;
    PieceType getType() override { return KNIGHT; };
  };
} // namespace chess_client
