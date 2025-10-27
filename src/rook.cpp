#include "rook.h"

namespace chess_client {
  Rook::Rook(Square* square, PieceColor color) : Piece(square, color) {
    loadSurface(color == BLACK ? "res/b_rook.png" : "res/w_rook.png");
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

  */
  std::vector<Move> Rook::getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) {
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
            moves.push_back({ getSquare()->pos, nextPos, board[posToIndex(nextPos)].occupyingPiece });
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

    return moves;
  };

  void Rook::performMove(std::array<Square, 64>& board, Move& move) {
    if (move.castlingRook && isValidPosition(move.castlingRookDst)) {
      setSquare(getSquareAtPosition(board, move.castlingRookDst));
      if (!hasMoved()) {
        setMoved(true);
        move.firstMove = true;
      }
      if (move.firstMove) { // We are undoing a piece move, reset the flag
        setMoved(false);
      }
    }
    else {
      Piece::performMove(board, move);
    }
  };
} // namespace chess_client