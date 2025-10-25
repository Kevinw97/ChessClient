#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
class Knight : public Piece {
public:
  Knight(Square *square, bool isBlack);
  std::vector<Position> getPossibleMoves(std::array<Square, 64>& board) override;
};
} // namespace chess_client
