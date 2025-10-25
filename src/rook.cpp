#include "rook.h"

namespace chess_client {
Rook::Rook(Square *square, bool isBlack) : Piece(square, isBlack) {
  loadSurface(isBlack ? "res/b_rook.png" : "res/w_rook.png");
};
/*
The rook's directions are denoted like so:
        _ 0 _
        3 _ 1
        _ 2 _
Rooks can't hop over pieces so we must break out as soon as we collide something 
      0: 0 -1
      1: 1  0
      2: 0  1
      3: -1 0
TODO: Castling
*/
std::vector<Position> Rook::getPossibleMoves(std::array<Square, 64>& board) {
  std::vector<Position> moves;
  int x = getPosition().x;
  int y = getPosition().y;
  auto addMovesForDirection = [&](Position direction) {
    for (int i = 1; i < 8; i++) {
      Position nextPos = {x + i * direction.x, y + i * direction.y};
      if (!isValidPosition(nextPos)) {
        return;
      }
      if (positionIsOccupied(board, nextPos)) {
        if (isOpposingPiece(*board[posToIndex(nextPos)].occupyingPiece)) {
          moves.push_back(nextPos);
        }
        return;
      }
      moves.push_back(nextPos);
    }
  };
  addMovesForDirection({0, -1});
  addMovesForDirection({1, 0});
  addMovesForDirection({0, 1});
  addMovesForDirection({-1, 0});

  return moves;
};
} // namespace chess_client