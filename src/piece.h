#pragma once
#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

namespace chess_client
{
  struct Position 
  {
    int x;
    int y;
  };
  enum PieceType : unsigned char
  {
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
  class Piece
  {
  private:
    Position m_Position;
    SDL_Surface* m_Surface;
    bool m_isBlack;
    bool m_isAlive;

  public:
    Piece(int x, int y, bool isBlack);
    virtual std::vector<Position> getPossibleMoves() = 0;
    void loadSurface(const char* filepath);
    void setIsAlive(bool isAlive) { m_isAlive = isAlive; }
    bool isAlive() { return m_isAlive; }
    bool isBlack() { return m_isBlack; }
    Position getPosition() { return m_Position; }
    SDL_Surface* getSurface() { return m_Surface; }
  };
}