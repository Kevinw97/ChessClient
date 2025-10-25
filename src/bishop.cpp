#include "bishop.h"
namespace chess_client {
Bishop::Bishop(int x, int y, bool isBlack)
    : Piece(x, y, isBlack) {
    loadSurface(isBlack ? "res/b_bishop.png" : "res/w_bishop.png");
};
/*
The diagonal directions are denoted like so:
          0 X 1
          X X X
          3 X 2
Bishops can't hop over pieces so we must break out as soon as we collide with something
         0: - -
         1: + -
         2: + +
         3: - +
*/
std::vector<Position> Bishop::getPossibleMoves(std::array<Square, 64>& board) {
    std::vector<Position> moves;
    int x = getPosition().x;
    int y = getPosition().y;
    auto addMovesForDirection = [&](Position direction) {
      for (int i = 1; i < 8; i++) {
        Position nextPos = {x + i * direction.x, y + i * direction.y};
        if (!isValidPosition(nextPos)) {
          return;
        }
        if (isPositionOccupied(board, nextPos)) {
          if (isOpposingPiece(*board[posToIndex(nextPos)].occupyingPiece)) {
            moves.push_back(nextPos);
          }
          return;
        }
        moves.push_back(nextPos);
      }
    };
    addMovesForDirection({-1, -1});
    addMovesForDirection({1, -1});
    addMovesForDirection({1, 1});
    addMovesForDirection({-1, 1});

    return moves;
};
} // namespace chess_client