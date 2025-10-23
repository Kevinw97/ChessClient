#include "chess_game.h"
#include "sdl_handler.h"
#include <thread>
#include <iostream>
namespace chess_client
{
  ChessGame::ChessGame()
  {
    RenderHandler renderHandler("Chess Game", 960, 960);
    renderHandler.init();

    // Generate pieces
    for (int i = 0; i < 8; i++)
    {
      m_WhitePieces[i] = std::make_shared<Pawn>(i, 6, false);
      m_BlackPieces[i] = std::make_shared<Pawn>(i, 1, true);
    }
    m_WhitePieces[8] = std::make_shared<Rook>(0, 7, false);
    m_WhitePieces[9] = std::make_shared<Rook>(7, 7, false);
    m_WhitePieces[10] = std::make_shared<Knight>(1, 7, false);
    m_WhitePieces[11] = std::make_shared<Knight>(6, 7, false);
    m_WhitePieces[12] = std::make_shared<Bishop>(2, 7, false);
    m_WhitePieces[13] = std::make_shared<Bishop>(5, 7, false);
    m_WhitePieces[14] = std::make_shared<Queen>(3, 7, false);
    m_WhitePieces[15] = std::make_shared<King>(4, 7, false);
    m_BlackPieces[8] = std::make_shared<Rook>(0, 0, true);
    m_BlackPieces[9] = std::make_shared<Rook>(7, 0, true);
    m_BlackPieces[10] = std::make_shared<Knight>(1, 0, true);
    m_BlackPieces[11] = std::make_shared<Knight>(6, 0, true);
    m_BlackPieces[12] = std::make_shared<Bishop>(2, 0, true);
    m_BlackPieces[13] = std::make_shared<Bishop>(5, 0, true);
    m_BlackPieces[14] = std::make_shared<Queen>(3, 0, true);
    m_BlackPieces[15] = std::make_shared<King>(4, 0, true);

    for (const auto& piece : m_WhitePieces)
    {
      renderHandler.placePiece(piece, piece->getPosition().x, piece->getPosition().y);
    }

    for (const auto& piece : m_BlackPieces)
    {
      renderHandler.placePiece(piece, piece->getPosition().x, piece->getPosition().y);
    }

    renderHandler.drawChessBoard();

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
  }
}  // namespace chess_client