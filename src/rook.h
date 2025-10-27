#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  class Rook : public Piece {
  private:
  public:
    Rook(Square* square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) override;
    std::string getType() override { return std::string("Rook"); };
    void performMove(std::array<Square, 64>& board, Move& move) override;
  };
} // namespace chess_client