#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
#include "chess.h"

namespace chess_client {
class Piece {
private:
    Position m_Position;
    SDL_Surface* m_Surface;
    bool m_isBlack;
    bool m_isAlive;

public:
    Piece(int x, int y, bool isBlack);
    void loadSurface(const char* filepath);
    void setIsAlive(bool isAlive) { m_isAlive = isAlive; }
    bool isAlive() { return m_isAlive; }
    bool isBlack() { return m_isBlack; }
    bool isOpposingPiece(Piece &piece);
    Position getPosition() { return m_Position; }
    void setPosition(int x, int y) {
        m_Position.x = x;
        m_Position.y = y;
    }
    SDL_Surface* getSurface() { return m_Surface; }
    virtual std::vector<Position> getPossibleMoves(std::array<Square, 64>& board) = 0;
};
} // namespace chess_client