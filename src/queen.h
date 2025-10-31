#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_online {
class Queen : public Piece {
public:
    Queen(Square *square, PieceColor color);
    std::vector<Move> getPossibleMoves(const std::array<Square, NUM_SQUARES> &board, const std::vector<Action> &actionHistory) override;
    PieceType getType() override { return QUEEN; };
};
} // namespace chess_online
