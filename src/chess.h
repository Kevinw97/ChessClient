#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <array>
#include <memory>
#include <Windows.h>
#include <cstdlib>

#define CHESS_LOGGING 1

#if CHESS_LOGGING
#define LOG_COUT(X) std::cout << X << std::endl
#define LOG_PRINTF(...) printf(__VA_ARGS__)
#else
#define LOG_COUT(X)
#define LOG_PRINTF(...)
#endif

namespace chess_client {

  const int BOARD_SIZE = 960;
  const int SQUARE_SIZE = BOARD_SIZE / 8;

  class Piece;

  struct Position {
    int x;
    int y;
  };

  struct Move {
    Position dst;
    std::shared_ptr<Piece> capturedPiece;
  };

  struct Action {
    std::shared_ptr<Piece> piece;
    struct Move move;
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
    RGBAColor Color;
    bool isHighlighted;
    std::shared_ptr<Piece> occupyingPiece;
  };

  enum PlayerColor : bool {
    WHITE,
    BLACK
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

  inline bool isValidPosition(Position pos) {
    return (pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8);
  }

  inline int posToIndex(Position pos) {
    if (!isValidPosition(pos)) {
      LOG_COUT("Invalid square position passed to posToIndex");
      return -1;
    }
    return pos.y * 8 + pos.x;
  }

  inline bool positionIsOccupied(const std::array<Square, 64>& board, const Position& pos) {
    return board[posToIndex(pos)].occupyingPiece != nullptr;
  }

  inline Square* getSquareAtPosition(std::array<Square, 64>& board, int x, int y) {
    int index = posToIndex({ x, y });
    if (index == -1) {
      return nullptr;
    }
    return &board[index];
  }
} // namespace chess_client