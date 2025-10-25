#include "sdl_handler.h"
#include <iostream>
#include <thread>

namespace chess_client {
RenderHandler::RenderHandler(const char* title, int width, int height)
    : m_Window(nullptr)
    , m_Renderer(nullptr)
    , m_Title(title)
    , m_Width(width)
    , m_Height(height) {
  init();
};

bool RenderHandler::init() {
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(m_Title, m_Width, m_Height, 0, &m_Window, &m_Renderer);
  SDL_RenderPresent(m_Renderer);
  return 0;
};

void RenderHandler::generateInitialBoard(std::array<Square, 64> &board) {
  float squareWidth = static_cast<float>(m_Width / 8);
  float squareHeight = static_cast<float>(m_Height / 8);
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int i = posToIndex({x, y});
      board[i].pos = {x, y};
      board[i].rect = {
          x * squareWidth, 
          y * squareHeight, 
          squareWidth, 
          squareHeight};
      float padding = getPadding(board[i]);
      board[i].innerRect = {
          x * squareWidth + padding, 
          y * squareHeight + padding,
          squareWidth - 2 * padding, 
          squareHeight - 2 * padding};
      board[i].color = {
          static_cast<Uint8>((x + y) % 2 == 0 ? 238 : 118),
          static_cast<Uint8>((x + y) % 2 == 0 ? 238 : 150),
          static_cast<Uint8>((x + y) % 2 == 0 ? 210 : 86), 255};
      board[i].isHighlighted = false;
    }
  }
}

void RenderHandler::drawChessBoard(std::array<Square, 64>& board) {
  // Set canvas to white
  SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
  SDL_RenderClear(m_Renderer);

  for (Square &square : board) {
    SDL_SetRenderDrawColor(m_Renderer, square.color.r, square.color.g,
                           square.color.b, square.color.a);
    SDL_RenderFillRect(m_Renderer, &square.rect);
    if (square.occupyingPiece) {
      SDL_Surface* pieceSurface = square.occupyingPiece->getSurface();
      if (!pieceSurface) {
        std::cerr << "Failed to load piece surface." << std::endl;
        throw std::runtime_error("Failed to load piece surface");
      }
      SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, pieceSurface);
      if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError()
                  << std::endl;
        throw std::runtime_error("Failed to load piece surface");
      }
      SDL_RenderTexture(m_Renderer, texture, nullptr, &square.innerRect);
      SDL_DestroyTexture(texture);
    }
    if (square.isHighlighted) {
      SDL_SetRenderDrawColor(m_Renderer, 0, 200, 255,
                             128); // Cyan with 50% transparency
      SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);
      SDL_RenderFillRect(m_Renderer, &square.rect);
    }
  }
  SDL_RenderPresent(m_Renderer);
}
} // namespace chess_client