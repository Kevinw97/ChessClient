#include "pawn.h"
#include "chess.h"

namespace chess_client {
  Pawn::Pawn(Square* square, bool isBlack) : Piece(square, isBlack) {
    loadSurface(isBlack ? "res/b_pawn.png" : "res/w_pawn.png");
  };

  std::vector<Move> Pawn::getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) {
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
      Position nextPos = { x, y + 2 * direction };
      if (isValidPosition(nextPos) && !positionIsOccupied(board, nextPos)) {
        moves.push_back({ nextPos, nullptr });
      }
    }
    Position frontPos = { x, y + direction };
    if (isValidPosition(frontPos) && !positionIsOccupied(board, frontPos)) {
      moves.push_back({ frontPos, nullptr });
    }
    Position leftDiagonal = { x - 1, y + direction };
    if (isValidPosition(leftDiagonal)) {
      // Regular takeover
      if (positionIsOccupied(board, leftDiagonal) &&
        isOpposingPiece(board[posToIndex(leftDiagonal)].occupyingPiece)) {
        moves.push_back({ leftDiagonal, board[posToIndex(leftDiagonal)].occupyingPiece });
      }
      // En passante
      if (m_RowsAdvanced == 3 && positionIsOccupied(board, { x - 1, y })) {
        const std::shared_ptr<Piece>& occupyingPiece = board[posToIndex({ x - 1, y })].occupyingPiece;
        if (isOpposingPiece(occupyingPiece) &&
          actionHistory.back().piece == occupyingPiece) {
          moves.push_back({ leftDiagonal, occupyingPiece });
        }
      }
    }
    Position rightDiagonal = { x + 1, y + direction };
    if (isValidPosition(rightDiagonal)) {
      // Regular takeover
      if (positionIsOccupied(board, rightDiagonal) &&
        isOpposingPiece(board[posToIndex(rightDiagonal)].occupyingPiece)) {
        moves.push_back(
          { rightDiagonal, board[posToIndex(rightDiagonal)].occupyingPiece });
      }
      // En passante
      if (m_RowsAdvanced == 3 && positionIsOccupied(board, { x + 1, y })) {
        const std::shared_ptr<Piece>& occupyingPiece = board[posToIndex({ x + 1, y })].occupyingPiece;
        if (isOpposingPiece(occupyingPiece) &&
          actionHistory.back().piece == occupyingPiece) {
          moves.push_back({ rightDiagonal, occupyingPiece });
        }
      }
    }
    return moves;
  };

  void Pawn::performMove(std::array<Square, 64>& board, const Move& move) {
    m_RowsAdvanced += std::abs(move.dst.y - getPosition().y);
    Piece::performMove(board, move);
  }
} // namespace chess_client