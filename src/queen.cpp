#include "queen.h"

namespace chess_client {
  Queen::Queen(Square* square, PieceColor color) : Piece(square, color) {
    loadSurface(color == BLACK ? "res/b_queen.png" : "res/w_queen.png");
  };

  // Queen literally moves like how a bishop + rook move
  std::vector<Move> Queen::getPossibleMoves(const std::array<Square, NUM_SQUARES>& board, const std::vector<Action>& actionHistory) {
    std::vector<Move> moves;
    if (!isAlive()) {
      return moves;
    }
    int x = getSquare()->x;
    int y = getSquare()->y;
    auto addMovesForDirection = [&](Position direction) {
      for (int i = 1; i < 8; i++) {
        Position nextPos = { x + i * direction.x, y + i * direction.y };
        if (!isValidPosition(nextPos)) {
          return;
        }
        if (positionIsOccupied(board, nextPos)) {
          if (isOpposingPiece(board[posToIndex(nextPos)].occupyingPiece)) {
            moves.push_back({ getSquare()->pos, nextPos, board[posToIndex(nextPos)].occupyingPiece});
          }
          return;
        }
        moves.push_back({ getSquare()->pos, nextPos });
      }
      };
    addMovesForDirection({ 0, -1 });
    addMovesForDirection({ 1, 0 });
    addMovesForDirection({ 0, 1 });
    addMovesForDirection({ -1, 0 });
    addMovesForDirection({ -1, -1 });
    addMovesForDirection({ 1, -1 });
    addMovesForDirection({ 1, 1 });
    addMovesForDirection({ -1, 1 });

    return moves;
  };
} // namespace chess_client