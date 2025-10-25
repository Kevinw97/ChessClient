#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
#include "chess.h"

namespace chess_client {
class Piece {
private:
    Square* m_Square;
    SDL_Surface* m_Surface;
    bool m_isBlack;
    bool m_isAlive;

public:
    Piece(Square* square, bool isBlack);
    void loadSurface(const char* filepath);
    void setIsAlive(bool isAlive) { m_isAlive = isAlive; }
    bool isAlive() { return m_isAlive; }
    bool isBlack() { return m_isBlack; }
    bool isOpposingPiece(Piece &piece);
    Position getPosition() { 
      return m_Square ? m_Square->pos : Position{-1, -1}; 
    }
    void setSquare(Square *square) { m_Square = square; }
    SDL_Surface* getSurface() { return m_Surface; }
    virtual std::vector<Position> getPossibleMoves(std::array<Square, 64>& board) = 0;
};
} // namespace chess_client