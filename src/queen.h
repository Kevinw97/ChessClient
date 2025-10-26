#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  class Queen : public Piece {
  public:
    Queen(Square* square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) override;
  };
} // namespace chess_client
