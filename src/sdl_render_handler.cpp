#include "sdl_render_handler.h"
#include <iostream>
#include <thread>

namespace chess_client {
  RenderHandler::RenderHandler(const char* title, int width, int height)
    : m_Window(nullptr)
    , m_Renderer(nullptr)
    , m_Title(title) {};

  bool RenderHandler::init() {
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    // Window is essentially a 10 * 8.5 Chess board, where the extra 2 columns are 
    // for rendering taken pieces and the extra row is for displaying game status
    SDL_CreateWindowAndRenderer(m_Title,
      static_cast<int>(BOARD_SIZE + CAPTURED_CONTAINER_SIZE * 4),
      static_cast<int>(BOARD_SIZE),
      0,
      &m_Window,
      &m_Renderer);
    SDL_RenderPresent(m_Renderer);
    return 0;
  };

  void RenderHandler::generateInitialBoard(std::array<Square, 64>& board) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        int i = posToIndex({ x, y });
        board[i].pos = { x, y };
        board[i].rect = {
          static_cast<float>(x * SQUARE_SIZE + SQUARE_SIZE),
          static_cast<float>(y * SQUARE_SIZE),
          static_cast<float>(SQUARE_SIZE),
          static_cast<float>(SQUARE_SIZE) };
        float padding = getPadding(board[i]);
        board[i].innerRect = {
            static_cast<float>(x * SQUARE_SIZE + padding + SQUARE_SIZE),
            static_cast<float>(y * SQUARE_SIZE + padding),
            static_cast<float>(SQUARE_SIZE - 2 * padding),
            static_cast<float>(SQUARE_SIZE - 2 * padding) };
        board[i].Color = {
            static_cast<Uint8>((x + y) % 2 == 0 ? 238 : 118),
            static_cast<Uint8>((x + y) % 2 == 0 ? 238 : 150),
            static_cast<Uint8>((x + y) % 2 == 0 ? 210 : 86), 255 };
        board[i].isHighlighted = false;
      }
    }
  }

  void RenderHandler::drawChessBoard(const std::array<Square, 64>& board) {
    // Set canvas to black
    SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_Renderer);

    for (const Square& square : board) {
      SDL_SetRenderDrawColor(m_Renderer, square.Color.r, square.Color.g,
        square.Color.b, square.Color.a);
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

    drawCapturedPieces();

    SDL_RenderPresent(m_Renderer);
  }

  void RenderHandler::capturePiece(const std::shared_ptr<Piece>& piece) {
    PieceColor color = piece->getColor();
    if (color == BLACK) {
      m_BlackPiecesCaptured.push_back({ getNextCaptureContainer(color), piece });
    }
    if (color == WHITE) {
      m_WhitePiecesCaptured.push_back({ getNextCaptureContainer(color), piece });
    }
  }

  void RenderHandler::undoCapture(PieceColor color) {
    if (color == BLACK) {
      m_BlackPiecesCaptured.pop_back();
    }
    if (color == WHITE) {
      m_WhitePiecesCaptured.pop_back();
    }
  }

  void RenderHandler::drawCapturedPieces() {
    for (const CapturedPiece& capturedPiece : m_BlackPiecesCaptured) {
      SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
      SDL_RenderFillRect(m_Renderer, &capturedPiece.container);
      SDL_Surface* pieceSurface = capturedPiece.piece->getSurface();
      SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, pieceSurface);
      SDL_RenderTexture(m_Renderer, texture, nullptr, &capturedPiece.container);
      SDL_DestroyTexture(texture);
    }
    for (const CapturedPiece& capturedPiece : m_WhitePiecesCaptured) {
      SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
      SDL_RenderFillRect(m_Renderer, &capturedPiece.container);
      SDL_Surface* pieceSurface = capturedPiece.piece->getSurface();
      SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, pieceSurface);
      SDL_RenderTexture(m_Renderer, texture, nullptr, &capturedPiece.container);
      SDL_DestroyTexture(texture);
    }
  }

  void RenderHandler::clearCapturedPieces() {
    m_WhitePiecesCaptured.clear();
    m_BlackPiecesCaptured.clear();
  }
  /*
  typedef struct SDL_FRect
  {
      float x;
      float y;
      float w;
      float h;
  } SDL_FRect;
  */
  SDL_FRect RenderHandler::getNextCaptureContainer(PieceColor color) {
    const std::vector<CapturedPiece>& pieces = color == WHITE ? m_WhitePiecesCaptured : m_BlackPiecesCaptured;
    float captureAreaWidth = SQUARE_SIZE;
    float captureAreaHeight = BOARD_SIZE;
    float captureAreaX = color == WHITE ? static_cast<float>(BOARD_SIZE + SQUARE_SIZE) : 0.0f;
    float captureAreaY = 0;
    size_t numPiecesCaptured = pieces.size();
    return {
      captureAreaX + (numPiecesCaptured % 2) * CAPTURED_CONTAINER_SIZE,
      captureAreaY + (numPiecesCaptured / 2) * CAPTURED_CONTAINER_SIZE,
      CAPTURED_CONTAINER_SIZE,
      CAPTURED_CONTAINER_SIZE
    };
  }

  Position RenderHandler::mouseToPosition(SDL_Event* event) {
    int mouseX = static_cast<int>(event->button.x);
    int mouseY = static_cast<int>(event->button.y);
    LOG_PRINTF("Mouse clicked at: (%d, %d)\n", mouseX, mouseY);
    int posX = (mouseX - SQUARE_SIZE) / SQUARE_SIZE;
    int posY = (mouseY) / SQUARE_SIZE;
    LOG_PRINTF("Returning pos: (%d, %d)\n", posX, posY);
    return { posX, posY };
  }
} // namespace chess_client