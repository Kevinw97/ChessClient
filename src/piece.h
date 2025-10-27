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
    Position m_InitialPosition;

  public:
    Piece(Square* square, PieceColor color);
    virtual ~Piece() = default;
    void loadSurface(const char* filepath);
    virtual void setIsAlive(bool isAlive) { m_isAlive = isAlive; }
    bool isAlive() const { return m_isAlive; }
    bool isOpposingPiece(const std::shared_ptr<Piece>& piece);
    PieceColor getColor() const { return m_Color; }
    Position getInitialPosition() { return m_InitialPosition; }
    Square* getSquare() { return m_Square; };
    bool hasMoved() { return m_HasMoved; };
    void setMoved(bool val) { m_HasMoved = val; }
    virtual void resetPiece(std::array<Square, 64>& board);
    virtual void setSquare(Square* square) { m_Square = square; };
    virtual SDL_Surface* getSurface() { return m_Surface; }
    virtual void performMove(std::array<Square, 64>& board, Move& move);
    virtual std::vector<Move> getPossibleMoves(const std::array<Square, 64>& board, const std::vector<Action>& actionHistory) = 0;
    virtual std::string getType() = 0;
  };
} // namespace chess_client