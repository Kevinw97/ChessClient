#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class King : public Piece {
private:
  bool m_inCheck;
public:
  King(Square *square, bool isBlack);
  std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board) override;
};
} // namespace chess_client