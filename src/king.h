#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_online {
class King : public Piece {
public:
    King(Square *square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, NUM_SQUARES> &board, const std::vector<Action> &actionHistory) override;
    PieceType getType() override { return KING; };
};
} // namespace chess_online