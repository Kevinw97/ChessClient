#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "piece.h"

namespace chess_client
{
  struct RGBColor
  {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
  };
  struct Square
  {
    SDL_FRect rect;
    RGBColor color;
    bool isHighlighted;
    std::shared_ptr<Piece> occupyingPiece;
  };
  class RenderHandler
  {
  private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    const char* m_Title;
    int m_Width;
    int m_Height;
    Square m_Squares[64];

    void generateSquares();
    

  public:
    RenderHandler(const char* title, int width, int height);
    bool init();
    void drawChessBoard();
    void placePiece(std::shared_ptr<Piece> piece, int x, int y);
    //void drawPieces(std::unique_ptr<Piece>(&whitePieces)[16], const std::unique_ptr<Piece>(&blackPieces)[16]);

  };
}  // namespace chess_client


