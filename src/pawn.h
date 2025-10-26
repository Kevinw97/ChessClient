#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  class Pawn : public Piece {
  private:
    int m_RowsAdvanced = 0;
  public:
    Pawn(Square* square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) override;
    void performMove(std::array<Square, 64>& board, const Position& pos);
  };
} // namespace chess_client