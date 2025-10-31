#include "pawn.h"
#include "bishop.h"
#include "chess.h"
#include "king.h"
#include "knight.h"
#include "queen.h"
#include "rook.h"

namespace chess_online {
Pawn::Pawn(Square *square, PieceColor color) : Piece(square, color) {
#ifdef CHESS_CLIENT_BUILD
    loadSurface(color == BLACK ? "res/b_pawn.png" : "res/w_pawn.png");
#endif
};

void Pawn::setIsAlive(bool isAlive) {
    if (m_PromotedPiece) {
        m_PromotedPiece->setIsAlive(isAlive);
    }
    Piece::setIsAlive(isAlive);
};

void Pawn::setSquare(Square *square) {
    if (m_PromotedPiece) {
        m_PromotedPiece->setSquare(square);
    }
    Piece::setSquare(square);
};

std::vector<Move> Pawn::getPossibleMoves(const std::array<Square, NUM_SQUARES> &board, const std::vector<Action> &actionHistory) {
    if (m_PromotedPiece) {
        return m_PromotedPiece->getPossibleMoves(board, actionHistory);
    }

    std::vector<Move> moves;
    if (!isAlive()) {
        return moves;
    }
    const int x = getSquare()->x;
    const int y = getSquare()->y;
    const int direction = getColor() == BLACK ? 1 : -1;
    const int startRow = getColor() == BLACK ? 1 : 6;

    Position frontPos = {x, y + direction};
    if (isValidPosition(frontPos) && !positionIsOccupied(board, frontPos)) {
        moves.push_back({getSquare()->pos, frontPos});
    }

    if (y == startRow) {
        // Initial double move
        Position nextPos = {x, y + 2 * direction};
        if (isValidPosition(nextPos) && !positionIsOccupied(board, frontPos) && !positionIsOccupied(board, nextPos)) {
            moves.push_back({getSquare()->pos, nextPos});
        }
    }

    Position leftDiagonal = {x - 1, y + direction};
    if (isValidPosition(leftDiagonal)) {
        // Regular takeover
        if (positionIsOccupied(board, leftDiagonal) &&
            isOpposingPiece(board[posToIndex(leftDiagonal)].occupyingPiece)) {
            moves.push_back({getSquare()->pos, leftDiagonal, board[posToIndex(leftDiagonal)].occupyingPiece});
        }
        // En passante
        if (m_RowsAdvanced == 3 && positionIsOccupied(board, {x - 1, y})) {
            const std::shared_ptr<Piece> &occupyingPiece = board[posToIndex({x - 1, y})].occupyingPiece;
            if (isOpposingPiece(occupyingPiece) &&
                actionHistory.back().piece == occupyingPiece) {
                moves.push_back({getSquare()->pos, leftDiagonal, occupyingPiece});
            }
        }
    }

    Position rightDiagonal = {x + 1, y + direction};
    if (isValidPosition(rightDiagonal)) {
        // Regular takeover
        if (positionIsOccupied(board, rightDiagonal) &&
            isOpposingPiece(board[posToIndex(rightDiagonal)].occupyingPiece)) {
            moves.push_back({getSquare()->pos, rightDiagonal, board[posToIndex(rightDiagonal)].occupyingPiece});
        }
        // En passante
        if (m_RowsAdvanced == 3 && positionIsOccupied(board, {x + 1, y})) {
            const std::shared_ptr<Piece> &occupyingPiece = board[posToIndex({x + 1, y})].occupyingPiece;
            if (isOpposingPiece(occupyingPiece) &&
                actionHistory.back().piece == occupyingPiece) {
                moves.push_back({getSquare()->pos, rightDiagonal, occupyingPiece});
            }
        }
    }

    return moves;
};

void Pawn::performMove(std::array<Square, NUM_SQUARES> &board, const Move &move) {
    if (m_PromotedPiece) {
        m_PromotedPiece->performMove(board, move);
    }
    if (m_RowsAdvanced < 6) {
        m_RowsAdvanced = getColor() == BLACK ? move.dst.y - getInitialPosition().y : getInitialPosition().y - move.dst.y;
    }

    Piece::performMove(board, move);

    if (move.promoteType) {
        promotePiece(move.promoteType);
    }
};

void Pawn::undoPromote() {
    m_PromotedPiece.reset();
}

bool Pawn::isPromoted() {
    return (m_PromotedPiece != nullptr);
}

void Pawn::promotePiece(PieceType type) {
    if (m_RowsAdvanced != 6 || m_PromotedPiece) {
        return;
    }
    if (type == PAWN || type == KING) {
        std::cerr << "Should not have called promote with this type\n";
        throw std::runtime_error("Wrong promotion type\n");
    }
    switch (type) {
    case ROOK: {
        m_PromotedPiece = std::make_unique<Rook>(getSquare(), getColor());
        break;
    }
    case BISHOP: {
        m_PromotedPiece = std::make_unique<Bishop>(getSquare(), getColor());
        break;
    }
    case KNIGHT: {
        m_PromotedPiece = std::make_unique<Knight>(getSquare(), getColor());
        break;
    }
    case QUEEN: {
        m_PromotedPiece = std::make_unique<Queen>(getSquare(), getColor());
        break;
    }
    default: {
        std::cerr << "Should not have called promote with this type\n";
        throw std::runtime_error("Wrong promotion type\n");
    }
    }
    m_PromotedPiece->setMoved(true);
    return;
}

bool Pawn::canPromote(const Move &move) {
    return (!m_PromotedPiece && getColor() == BLACK ? (move.dst.y - getInitialPosition().y == 6) : (getInitialPosition().y - move.dst.y == 6));
}

#ifdef CHESS_CLIENT_BUILD
SDL_Surface *Pawn::getSurface() {
    if (m_PromotedPiece) {
        return m_PromotedPiece->getSurface();
    } else {
        return Piece::getSurface();
    }
};
#endif

void Pawn::resetPiece(std::array<Square, NUM_SQUARES> &board) {
    m_PromotedPiece.reset();
    Piece::resetPiece(board);
};
} // namespace chess_online