#include "piece.h"

namespace chess_client {
  Piece::Piece(Square* square, PieceColor color)
    : m_InitialPosition(square->pos)
    , m_Square(square)
    , m_Color(color)
    , m_Surface(nullptr) {};

  void Piece::loadSurface(const char* filepath) {
    m_Surface = IMG_Load(filepath);
    if (!m_Surface) {
      std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
      throw std::runtime_error("Failed to load piece surface");
    }
  }

  void Piece::performMove(std::array<Square, 64>& board, Move& move) {
    setSquare(getSquareAtPosition(board, move.dst));
    if (!hasMoved()) {
      setMoved(true);
      move.firstMove = true;
    }
    else { // We are undoing a piece move, reset the flag
      setMoved(false);
    }
  }

  bool Piece::isOpposingPiece(const std::shared_ptr<Piece>& piece) {
    return piece->getColor() != getColor();
  }

  void Piece::resetPiece(std::array<Square, 64>& board) {
    Square* square = getSquareAtPosition(board, m_InitialPosition);
    setSquare(square);
    setIsAlive(true);
    setMoved(false);
  }
} // namespace chess_client
