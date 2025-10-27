#include "bishop.h"
namespace chess_client {
  Bishop::Bishop(Square* square, PieceColor color) : Piece(square, color) {
    loadSurface(color == BLACK ? "res/b_bishop.png" : "res/w_bishop.png");
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
  std::vector<Move>
    Bishop::getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) {

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
        moves.push_back({ getSquare()->pos, nextPos});
      }
      };
    addMovesForDirection({ -1, -1 });
    addMovesForDirection({ 1, -1 });
    addMovesForDirection({ 1, 1 });
    addMovesForDirection({ -1, 1 });

    return moves;
  };
} // namespace chess_client