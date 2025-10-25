#pragma once
#include "chess.h"
#include "piece.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <array>

namespace chess_client {
const float RECT_PADDING = 10.0f;

class RenderHandler {

private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    const char* m_Title;
    int m_Width;
    int m_Height;

public:
    RenderHandler(const char* title, int width, int height);
    bool init();
    void drawChessBoard(std::array<Square, 64>& board);
    void generateSquares(std::array<Square, 64>& board);
    float getPadding(Square square) const { return 0.10f * square.rect.w; }
    Square* getSquareAtPosition(int x, int y);
    void unselectAllSquares();
};
} // namespace chess_client
