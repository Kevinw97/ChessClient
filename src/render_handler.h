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

  public:
    RenderHandler(const char* title, int width, int height);
    bool init();
    void drawChessBoard(const std::array<Square, 64>& board);
    void capturePiece(const std::shared_ptr<Piece>& piece);
    void drawCapturedPieces();
    void clearCapturedPieces();
    SDL_FRect getNextCaptureContainer(PieceColor color);
    void generateInitialBoard(std::array<Square, 64>& board);
    void unselectAllSquares();
    float getPadding(const Square& square) const { return 0.10f * square.rect.w; }
    Position mouseToPosition(SDL_Event* event);
  };
} // namespace chess_client
