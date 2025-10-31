#pragma once
#include "chess.h"
#include "piece.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <array>
#include <set>

namespace chess_client {
  const float RECT_PADDING = 10.0f;
  const float CAPTURED_CONTAINER_SIZE = SQUARE_SIZE / 2;

  struct CapturedPiece {
    SDL_FRect container;
    std::shared_ptr<Piece> piece;
  };

  struct MousePosition {
    float x;
    float y;
  };

  class RenderHandler {
  private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    const char* m_Title;
    std::vector<CapturedPiece> m_BlackPiecesCaptured;
    std::vector<CapturedPiece> m_WhitePiecesCaptured;
    float m_Width = 0;
    float m_Height = 0;
    bool m_Rotated = false;

  public:
    RenderHandler(const char* title);
    bool init();
    void drawChessBoard(const std::array<Square, NUM_SQUARES>& board);
    void capturePiece(const std::shared_ptr<Piece>& piece);
    void undoCapture(PieceColor color);
    void drawCapturedPieces();
    void clearCapturedPieces();
    SDL_FRect getNextCaptureContainer(PieceColor color);
    void generateInitialBoard(std::array<Square, NUM_SQUARES>& board);
    void unselectAllSquares();
    void rotateBoard(std::array<Square, NUM_SQUARES>& board);
    float getPadding(const Square& square) const { return 0.10f * square.rect.w; }
    Position mouseToPosition(SDL_Event* event);
  };
} // namespace chess_client
