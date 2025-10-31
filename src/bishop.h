#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_online {
class Bishop : public Piece {
public:
    Bishop(Square *square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, NUM_SQUARES> &board, const std::vector<Action> &actionHistory) override;
    PieceType getType() override { return BISHOP; };
};
} // namespace chess_online