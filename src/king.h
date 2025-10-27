#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  class King : public Piece {
  public:
    King(Square* square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) override;
    std::string getType() override { return std::string("King"); };
  };
} // namespace chess_client