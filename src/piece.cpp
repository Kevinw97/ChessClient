#include "piece.h"

namespace chess_client {
  Piece::Piece(Square* square, bool isBlack)
    : m_Square(square), m_isBlack(isBlack), m_isAlive(true),
    m_Surface(nullptr) {
  };

  void Piece::loadSurface(const char* filepath) {
    m_Surface = IMG_Load(filepath);
    if (!m_Surface) {
      std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
      throw std::runtime_error("Failed to load piece surface");
    }
  }

  void Piece::performMove(std::array<Square, 64>& board, const Move& move) {
    setSquare(getSquareAtPosition(board, move.dst.x, move.dst.y));
  }

  bool Piece::isOpposingPiece(const std::shared_ptr<Piece>& piece) {
    return piece->isBlack() != m_isBlack;
  }
} // namespace chess_client
