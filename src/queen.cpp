#include "queen.h"

namespace chess_client {
Queen::Queen(Square *square, bool isBlack) : Piece(square, isBlack) {
  loadSurface(isBlack ? "res/b_queen.png" : "res/w_queen.png");
};

// Queen literally moves like how a bishop + rook move
std::vector<Move> Queen::getPossibleMoves(const std::array<Square, 64>& board) {
  std::vector<Move> moves;
  if (!isAlive()) {
    return moves;
  }
  int x = getPosition().x;
  int y = getPosition().y;
  auto addMovesForDirection = [&](Position direction) {
    for (int i = 1; i < 8; i++) {
      Position nextPos = {x + i * direction.x, y + i * direction.y};
      if (!isValidPosition(nextPos)) {
        return;
      }
      if (positionIsOccupied(board, nextPos)) {
        if (isOpposingPiece(board[posToIndex(nextPos)].occupyingPiece)) {
          moves.push_back({nextPos, board[posToIndex(nextPos)].occupyingPiece});
        }
        return;
      }
      moves.push_back({nextPos, nullptr});
    }
  };
  addMovesForDirection({0, -1});
  addMovesForDirection({1, 0});
  addMovesForDirection({0, 1});
  addMovesForDirection({-1, 0});
  addMovesForDirection({-1, -1});
  addMovesForDirection({1, -1});
  addMovesForDirection({1, 1});
  addMovesForDirection({-1, 1});

  return moves;
};
} // namespace chess_client