#include "chess_game.h"

#include <iostream>
#include <thread>

#include "sdl_handler.h"
namespace chess_client {
const int WINDOW_SIZE = 960;
const int SQUARE_SIZE = WINDOW_SIZE / 8;

ChessGame::ChessGame()
    : m_RenderHandler("Chess Game", WINDOW_SIZE, WINDOW_SIZE)
    , m_Board{} {
  setupInitialPieces();
  m_RenderHandler.generateSquares(m_Board);
  m_RenderHandler.drawChessBoard(m_Board);
}

void ChessGame::setupInitialPieces() {
  // Generate pieces
  for (int i = 0; i < 8; i++) {
    m_WhitePieces.insert(std::make_shared<Pawn>(i, 6, false));
    m_BlackPieces.insert(std::make_shared<Pawn>(i, 1, true));
  }
  m_WhitePieces.insert(std::make_shared<Rook>(0, 7, false));
  m_WhitePieces.insert(std::make_shared<Rook>(7, 7, false));
  m_WhitePieces.insert(std::make_shared<Knight>(1, 7, false));
  m_WhitePieces.insert(std::make_shared<Knight>(6, 7, false));
  m_WhitePieces.insert(std::make_shared<Bishop>(2, 7, false));
  m_WhitePieces.insert(std::make_shared<Bishop>(5, 7, false));
  m_WhitePieces.insert(std::make_shared<Queen>(3, 7, false));
  m_WhitePieces.insert(std::make_shared<King>(4, 7, false));
  m_BlackPieces.insert(std::make_shared<Rook>(0, 0, true));
  m_BlackPieces.insert(std::make_shared<Rook>(7, 0, true));
  m_BlackPieces.insert(std::make_shared<Knight>(1, 0, true));
  m_BlackPieces.insert(std::make_shared<Knight>(6, 0, true));
  m_BlackPieces.insert(std::make_shared<Bishop>(2, 0, true));
  m_BlackPieces.insert(std::make_shared<Bishop>(5, 0, true));
  m_BlackPieces.insert(std::make_shared<Queen>(3, 0, true));
  m_BlackPieces.insert(std::make_shared<King>(4, 0, true));

  for (const auto &piece : m_WhitePieces) {
    int index = posToIndex({piece->getPosition().x, piece->getPosition().y});
    m_Board[index].occupyingPiece = piece;
  }

  for (const auto &piece : m_BlackPieces) {
    int index = posToIndex({piece->getPosition().x, piece->getPosition().y});
    m_Board[index].occupyingPiece = piece;
  }
}

void ChessGame::gameLoop() {
  SDL_Event event;
  while (SDL_WaitEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      return;
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (!isCurrentPlayersTurn()) {
        break;
      }
      handleMouseClick(&event);
    }
    }

    m_RenderHandler.drawChessBoard(m_Board);
  }
}

void ChessGame::run() {
  gameLoop(); 
}

Square* ChessGame::getSquareAtPosition(int x, int y) {
  return &m_Board[posToIndex({x, y})];
}

void ChessGame::handleMouseClick(SDL_Event* event) {
  // Only handle click events for current player's turn
  int mouseX = static_cast<int>(event->button.x), mouseY = static_cast<int>(event->button.y);
  std::cout << "Mouse Clicked at: (" << mouseX << ", " << mouseY << ")\n";
  int posX = mouseX / SQUARE_SIZE;
  int posY = mouseY / SQUARE_SIZE;

  if (!m_SelectedSquare) {
    selectSource(posX, posY);
  } else {
    selectDestination(posX, posY);
  }
}

void ChessGame::selectSource(int x, int y) {
  Square* clickedSquare = getSquareAtPosition(x, y);
  if (!clickedSquare) {
    std::cerr << "Clicked outside of board boundaries.\n";
    throw std::runtime_error("Clicked outside of board boundaries");
  }
  if (clickedSquare->occupyingPiece == nullptr) {
    std::cout << "You must select one of your own pieces\n";
    return;
  }
  if (clickedSquare->occupyingPiece->isBlack() && !m_PlayerIsBlack ||
      !clickedSquare->occupyingPiece->isBlack() && m_PlayerIsBlack) {
    std::cout << "You clicked on an opponent's piece.\n";
    return;
  }
  m_SelectedSquare = clickedSquare;
  clickedSquare->isHighlighted = true;

  // TODO: Highlight possible moves, implement taking pieces
  std::vector<Position> possibleMoves = clickedSquare->occupyingPiece->getPossibleMoves(m_Board);
  for (const Position &pos : possibleMoves) {
    Square* targetSquare = getSquareAtPosition(pos.x, pos.y);
    if (!targetSquare) {
      std::cerr << "No target square found\n";
      throw std::runtime_error("No target square found\n");
    }
    targetSquare->isHighlighted = true;
  }
}

void ChessGame::selectDestination(int x, int y) {
  Square* clickedSquare = getSquareAtPosition(x, y);
  if (!clickedSquare) {
    std::cerr << "Clicked outside of board boundaries.\n";
    throw std::runtime_error("Clicked outside of board boundaries");
    return;
  }

  // TODO: Move piece logic

  // Deselect piece after move attempt
  if (clickedSquare->isHighlighted && clickedSquare != m_SelectedSquare) {
    std::cout << "Moving piece to new square.\n";
    processMove(m_SelectedSquare->x, m_SelectedSquare->y, clickedSquare->x, clickedSquare->y);
  }
  unselectAllSquares();
  m_SelectedSquare = nullptr;
  return;
}

void ChessGame::processMove(int srcX, int srcY, int dstX, int dstY) {
  Square* srcSquare = getSquareAtPosition(srcX, srcY);
  Square* dstSquare = getSquareAtPosition(dstX, dstY);

  std::shared_ptr<Piece> srcPiece = srcSquare->occupyingPiece;
  std::shared_ptr<Piece> dstPiece = dstSquare->occupyingPiece;

  printf("Moving from (%d, %d) to (%d, %d)", srcX, srcY, dstX, dstY);

  if (srcPiece) {
    srcPiece->setPosition(dstX, dstY);
    dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);
  }
  if (dstPiece) {
    dstPiece->setIsAlive(false);
  }

  // Switch turn
  m_IsBlackTurn = !m_IsBlackTurn;
  // For now, switch players, until 2p capabilities or bot capabilities added
  m_PlayerIsBlack = !m_PlayerIsBlack;
}

bool ChessGame::isCurrentPlayersTurn() {
  return !((m_PlayerIsBlack && !m_IsBlackTurn) ||
           (!m_PlayerIsBlack && m_IsBlackTurn));
}

void ChessGame::unselectAllSquares() {
  for (Square &square : m_Board) {
    square.isHighlighted = false;
  }
}
} // namespace chess_client