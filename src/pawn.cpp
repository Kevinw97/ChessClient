#include "pawn.h"
#include "chess.h"

namespace chess_client {
Pawn::Pawn(int x, int y, bool isBlack) : Piece(x, y, isBlack) {
  loadSurface(isBlack ? "res/b_pawn.png" : "res/w_pawn.png");
};
std::vector<Position> Pawn::getPossibleMoves(std::array<Square, 64>& board) {
  std::vector<Position> moves;
  int x = getPosition().x;
  int y = getPosition().y;
  int direction = isBlack() ? 1 : -1;
  int startRow = isBlack() ? 1 : 6;
  if (getPosition().y == startRow) {
    // Initial double move
    Position nextPos = {x, y + 2 * direction};
    if (isValidPosition(nextPos) && !isPositionOccupied(board, nextPos)) {
      moves.push_back(nextPos);
    }
  }
  Position frontPos = {x, y + direction};
  if (isValidPosition(frontPos) && !isPositionOccupied(board, frontPos)) {
    moves.push_back(frontPos);
  }
  Position leftDiagonal = {x - 1, y + direction};
  std::cout << "Checking left diagonal" << std::endl;
  if (isValidPosition(leftDiagonal) && isPositionOccupied(board, leftDiagonal) &&
      isOpposingPiece(*board[posToIndex(leftDiagonal)].occupyingPiece)) {
    std::cout << "A" << std::endl;
    moves.push_back(leftDiagonal);
  }
  Position rightDiagonal = {x + 1, y + direction};
  std::cout << "Checking right diagonal" << std::endl;
  if (isValidPosition(rightDiagonal) && isPositionOccupied(board, rightDiagonal) &&
      isOpposingPiece(*board[posToIndex(rightDiagonal)].occupyingPiece)) {
    std::cout << "B" << std::endl;
    moves.push_back(rightDiagonal);
  }
  return moves;
};
} // namespace chess_client