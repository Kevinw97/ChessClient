#include "pawn.h"
#include "chess.h"

namespace chess_client {
Pawn::Pawn(Square *square, bool isBlack) : Piece(square, isBlack) {
  loadSurface(isBlack ? "res/b_pawn.png" : "res/w_pawn.png");
};
std::vector<Move> Pawn::getPossibleMoves(const std::array<Square, 64>& board) {
  std::vector<Move> moves;
  if (!isAlive()) {
    return moves;
  }
  int x = getPosition().x;
  int y = getPosition().y;
  int direction = isBlack() ? 1 : -1;
  int startRow = isBlack() ? 1 : 6;
  if (getPosition().y == startRow) {
    // Initial double move
    Position nextPos = {x, y + 2 * direction};
    if (isValidPosition(nextPos) && !positionIsOccupied(board, nextPos)) {
      moves.push_back({nextPos, nullptr});
    }
  }
  Position frontPos = {x, y + direction};
  if (isValidPosition(frontPos) && !positionIsOccupied(board, frontPos)) {
    moves.push_back({frontPos, nullptr});
  }
  Position leftDiagonal = {x - 1, y + direction};
  if (isValidPosition(leftDiagonal) && positionIsOccupied(board, leftDiagonal) &&
      isOpposingPiece(board[posToIndex(leftDiagonal)].occupyingPiece)) {
    moves.push_back({leftDiagonal, board[posToIndex(leftDiagonal)].occupyingPiece});
  }
  Position rightDiagonal = {x + 1, y + direction};
  if (isValidPosition(rightDiagonal) && positionIsOccupied(board, rightDiagonal) &&
      isOpposingPiece(board[posToIndex(rightDiagonal)].occupyingPiece)) {
    moves.push_back({rightDiagonal, board[posToIndex(rightDiagonal)].occupyingPiece});
  }
  return moves;
};
} // namespace chess_client