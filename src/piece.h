#pragma once
#include "chess.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>

namespace chess_client {
  class Piece {
  private:
    Square* m_Square;
    SDL_Surface* m_Surface;
    PieceColor m_Color;
    bool m_isAlive = true;
    bool m_HasMoved = false;

  public:
    Piece(Square* square, PieceColor color);
    void loadSurface(const char* filepath);
    void setIsAlive(bool isAlive) { m_isAlive = isAlive; }
    bool isAlive() const { return m_isAlive; }
    PieceColor getColor() const { return m_Color; }
    bool isOpposingPiece(const std::shared_ptr<Piece>& piece);
    Position getPosition() {
      return m_Square ? m_Square->pos : Position{ -1, -1 };
    }
    void setSquare(Square* square) { m_Square = square; }
    bool hasMoved() { return m_HasMoved; };
    SDL_Surface* getSurface() { return m_Surface; }
    virtual void performMove(std::array<Square, 64>& board, const Position& pos);
    virtual std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) = 0;
  };
} // namespace chess_client