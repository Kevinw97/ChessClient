#include "king.h"
#include "rook.h"

namespace chess_online {
King::King(Square *square, PieceColor color) : Piece(square, color) {
#ifdef CHESS_CLIENT_BUILD
    loadSurface(color == BLACK ? "res/b_king.png" : "res/w_king.png");
#endif
};

/*
    0 1 2
    7 K 3
    6 5 4
*/
std::vector<Move> King::getPossibleMoves(const std::array<Square, NUM_SQUARES> &board, const std::vector<Action> &actionHistory) {
    std::vector<Move> moves;
    if (!isAlive()) {
        return moves;
    }
    int x = getSquare()->x;
    int y = getSquare()->y;
    Position possibleMoves[] = {
        {-1, -1}, // 0
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
            if (isOpposingPiece(board[posToIndex(nextPos)].occupyingPiece)) {
                moves.push_back({getSquare()->pos, nextPos, board[posToIndex(nextPos)].occupyingPiece});
            }
            continue;
        }
        moves.push_back({getSquare()->pos, nextPos});
    }

    // Castling check: Queen side black rook will be at {0, 0}, king side will be at {7, 0}
    //                 Queen side white rook will be at {0, 7}, king side will be at {7, 7}
    // There must not be any pieces between the King and Rook as well
    if (!hasMoved()) {
        Position queenSideRookPos = {0, getColor() == BLACK ? 0 : 7};
        bool queenSideEmpty = true;
        for (int i = 1; i < x; i++) {
            if (positionIsOccupied(board, {i, getColor() == BLACK ? 0 : 7})) {
                queenSideEmpty = false;
                break;
            }
        }
        if (queenSideEmpty && positionIsOccupied(board, queenSideRookPos)) {
            const Square &queenSideSquare = board[posToIndex(queenSideRookPos)];
            if (std::dynamic_pointer_cast<Rook>(queenSideSquare.occupyingPiece) && !queenSideSquare.occupyingPiece->hasMoved()) {
                moves.push_back({getSquare()->pos,
                                 {x - 2, y},
                                 nullptr,
                                 queenSideRookPos,
                                 {x - 1, y},
                                 queenSideSquare.occupyingPiece});
            }
        }

        Position kingSideRookPos = {7, getColor() == BLACK ? 0 : 7};
        bool kingSideEmpty = true;
        for (int i = x + 1; i < 7; i++) {
            if (positionIsOccupied(board, {i, getColor() == BLACK ? 0 : 7})) {
                kingSideEmpty = false;
                break;
            }
        }
        if (kingSideEmpty && positionIsOccupied(board, kingSideRookPos)) {
            const Square &kingSideSquare = board[posToIndex(kingSideRookPos)];
            if (std::dynamic_pointer_cast<Rook>(kingSideSquare.occupyingPiece) && !kingSideSquare.occupyingPiece->hasMoved()) {
                moves.push_back({getSquare()->pos,
                                 {x + 2, y},
                                 nullptr,
                                 kingSideRookPos,
                                 {x + 1, y},
                                 kingSideSquare.occupyingPiece});
            }
        }
    }

    return moves;
};
} // namespace chess_online