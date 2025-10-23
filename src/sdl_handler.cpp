#include "sdl_handler.h"
#include <iostream>
#include <thread>

namespace chess_client
{
  RenderHandler::RenderHandler(const char* title, int width, int height)
    : m_Window(nullptr)
    , m_Renderer(nullptr)
    , m_Title(title)
    , m_Width(width)
    , m_Height(height)
    , m_Squares{}
  {
    init();
  };


  bool RenderHandler::init()
  {
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(m_Title, m_Width, m_Height, 0, &m_Window, &m_Renderer);

    generateSquares();
    drawChessBoard();

    //SDL_Surface* pieceSurface = IMG_Load("C:/Dev/ChessClient/ChessClient/res/b_bishop.png");
    //if (!pieceSurface)
    //{
    //  std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
    //}
    //SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, pieceSurface);
    //if (!texture)
    //{
    //  std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    //}
    //SDL_RenderTexture(m_Renderer, texture, nullptr, &rect);

    SDL_RenderPresent(m_Renderer);
    return 0;
  };

  void RenderHandler::generateSquares()
  {
    float squareWidth = m_Width / 8.0f;
    float squareHeight = m_Height / 8.0f;
    for (int y = 0; y < 8; y++)
    {
      for (int x = 0; x < 8; x++)
      {
        m_Squares[y * 8 + x].rect =
        {
            x * squareWidth,
            y * squareHeight,
            squareWidth,
            squareHeight
        };
        m_Squares[y * 8 + x].color =
        {
            static_cast<Uint8>((x + y) % 2 == 0 ? 238 : 118),
            static_cast<Uint8>((x + y) % 2 == 0 ? 238 : 150),
            static_cast<Uint8>((x + y) % 2 == 0 ? 210 : 86),
            255
        };
        m_Squares[y * 8 + x].isHighlighted = false;
      }
    }
  }

  void RenderHandler::drawChessBoard()
  {
    // Set canvas to white
    SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_Renderer);

    for (Square& square : m_Squares)
    {
      SDL_SetRenderDrawColor(m_Renderer, square.color.r, square.color.g, square.color.b, square.color.a);
      SDL_RenderFillRect(m_Renderer, &square.rect);
      if (square.occupyingPiece)
      {
        std::cout << "OCCUPYING PIECE DETECTED" << std::endl;
        SDL_Surface* pieceSurface = square.occupyingPiece->getSurface();
        if (!pieceSurface)
        {
          std::cerr << "Failed to load piece surface." << std::endl;
          throw std::runtime_error("Failed to load piece surface");
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, pieceSurface);
        if (!texture)
        {
          std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
          throw std::runtime_error("Failed to load piece surface");
        }
        SDL_RenderTexture(m_Renderer, texture, nullptr, &square.rect);
      }
    }
    SDL_RenderPresent(m_Renderer);
  }

  void RenderHandler::placePiece(std::shared_ptr<Piece> piece, int x, int y)
  {
    std::cout << "Placing piece at (" << x << ", " << y << ")" << std::endl;
    int index = y * 8 + x;
    m_Squares[index].occupyingPiece = piece;
  }
}