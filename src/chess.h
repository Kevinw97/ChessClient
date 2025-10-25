#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <array>
#include <memory>

namespace chess_client {
class Piece;
struct Position {
  int x;
  int y;
};
struct RGBAColor {
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
};
struct Square {
  union {
    Position pos;
    struct {
      int x;
      int y;
    };
  };
  SDL_FRect rect;
  SDL_FRect innerRect;
  RGBAColor color;
  bool isHighlighted;
  std::shared_ptr<Piece> occupyingPiece;
};
enum PieceType : unsigned char {
  WHITE_PAWN,
  WHITE_ROOK,
  WHITE_KNIGHT,
  WHITE_BISHOP,
  WHITE_QUEEN,
  WHITE_KING,
  BLACK_PAWN,
  BLACK_ROOK,
  BLACK_KNIGHT,
  BLACK_BISHOP,
  BLACK_QUEEN,
  BLACK_KING
};
static bool isValidPosition(int x, int y) {
  return (x >= 0 && x < 8 && y >= 0 && y < 8);
}
static int posToIndex(int x, int y) {
  if (!isValidPosition(x, y)) {
    std::cout << "Invalid position passed to posToIndex" << std::endl;
    return -1;
  }
  return y * 8 + x;
}
} // namespace chess_client