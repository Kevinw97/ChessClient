#pragma once
#include "chess.h"
#include "piece.h"

namespace chess_client {
  class Pawn : public Piece {
  private:
    int m_RowsAdvanced = 0;
    std::unique_ptr<Piece> m_PromotedPiece;
  public:
    Pawn(Square* square, PieceColor color);
    void setIsAlive(bool isAlive) override;
    void setSquare(Square* square) override;
    std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) override;
    void performMove(std::array<Square, 64>& board, Move& pos) override;
    SDL_Surface* getSurface() override;
    void promotePiece(PieceType type);
    void resetPiece(std::array<Square, 64>& board) override;
    std::string getType() { return std::string("Pawn"); };
  };
} // namespace chess_client