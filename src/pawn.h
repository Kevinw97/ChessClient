#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_online {
class Pawn : public Piece {
private:
    int m_RowsAdvanced = 0;
    std::unique_ptr<Piece> m_PromotedPiece;

public:
    Pawn(Square *square, PieceColor color);
    void setIsAlive(bool isAlive) override;
    void setSquare(Square *square) override;
    std::vector<Move> getPossibleMoves(const std::array<Square, NUM_SQUARES> &board, const std::vector<Action> &actionHistory) override;
    void performMove(std::array<Square, NUM_SQUARES> &board, const Move &pos) override;
#ifdef CHESS_CLIENT_BUILD
    SDL_Surface *getSurface() override;
#endif
    void promotePiece(PieceType type);
    bool canPromote(const Move &move);
    void undoPromote();
    void resetPiece(std::array<Square, NUM_SQUARES> &board) override;
    bool isPromoted();
    PieceType getType() override { return PAWN; };
};
} // namespace chess_online