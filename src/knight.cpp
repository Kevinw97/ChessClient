#include "knight.h"

namespace chess_client {
Knight::Knight(Square *square, bool isBlack) : Piece(square, isBlack) {
  loadSurface(isBlack ? "res/b_knight.png" : "res/w_knight.png");
};

/*
    _ 0 _ 1 _
    7 _ _ _ 2
    _ _ K _ _
    6 _ _ _ 3
    _ 5 _ 4 _
*/
std::vector<Position> Knight::getPossibleMoves(std::array<Square, 64> &board) {
  std::vector<Position> moves;
  int x = getPosition().x;
  int y = getPosition().y;
  Position possibleMoves[] = {{-1, -2},  // 0
                              {1, -2},   // 1
                              {2, -1},   // 2
                              {2, 1},    // 3
                              {1, 2},    // 4
                              {-1, 2},   // 5
                              {-2, 1},   // 6
                              {-2, -1}}; // 7
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