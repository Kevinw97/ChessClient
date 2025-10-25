#include "piece.h"

namespace chess_client {
Piece::Piece(Square *square, bool isBlack)
    : m_Square(square), m_isBlack(isBlack), m_isAlive(true),
      m_Surface(nullptr) {};

void Piece::loadSurface(const char* filepath) {
  m_Surface = IMG_Load(filepath);
  if (!m_Surface) {
    std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
    throw std::runtime_error("Failed to load piece surface");
  }
}

bool Piece::isOpposingPiece(Piece& piece) {
  return piece.isBlack() != m_isBlack;
}
} // namespace chess_client
