#include "king.h"

namespace chess_client {
King::King(Square *square, bool isBlack) : Piece(square, isBlack) {
    loadSurface(isBlack ? "res/b_king.png" : "res/w_king.png");
};

/*
    0 1 2
    7 K 3
    6 5 4
*/
std::vector<Position> King::getPossibleMoves(std::array<Square, 64>& board) {
  std::vector<Position> moves;
  int x = getPosition().x;
  int y = getPosition().y;
  Position possibleMoves[] = {{-1, -1}, // 0
                              {0, -1},  // 1
                              {1, -1},  // 2
                              {1, 0},   // 3
                              {1, 1},   // 4
                              {0, 1},   // 5
                              {-1, 1},  // 6
                              {-1, 0}}; // 7
  for (Position &move : possibleMoves) {
    Position nextPos = {x + move.x, y + move.y};
    if (!isValidPosition(nextPos)) {
      continue;
    }
    if (positionIsOccupied(board, nextPos)) {
      if (!isOpposingPiece(*board[posToIndex(nextPos)].occupyingPiece)) {
        continue;
      }
    }
    moves.push_back(nextPos);
  }
  return moves;
};
} // namespace chess_client