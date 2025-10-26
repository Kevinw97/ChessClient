#include "piece.h"

namespace chess_client {
  Piece::Piece(Square* square, PieceColor color)
    : m_Square(square), m_Color(color), m_Surface(nullptr) {
  };

  void Piece::loadSurface(const char* filepath) {
    m_Surface = IMG_Load(filepath);
    if (!m_Surface) {
      std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
      throw std::runtime_error("Failed to load piece surface");
    }
  }

  void Piece::performMove(std::array<Square, 64>& board, const Position& pos) {
    m_HasMoved = true;
    setSquare(getSquareAtPosition(board, pos.x, pos.y));
  }

  bool Piece::isOpposingPiece(const std::shared_ptr<Piece>& piece) {
    return piece->getColor() != getColor();
  }
} // namespace chess_client
