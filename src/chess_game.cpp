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
  m_RenderHandler.generateInitialBoard(m_Board);
  setupInitialPieces(m_Board);
  m_RenderHandler.drawChessBoard(m_Board);
}

void ChessGame::setupInitialPieces(std::array<Square, 64> &board) {
  // Generate pieces
  for (int i = 0; i < 8; i++) {
    m_WhitePieces.insert(std::make_shared<Pawn>(&board[posToIndex({i, 6})], false));
    m_BlackPieces.insert(std::make_shared<Pawn>(&board[posToIndex({i, 1})], true));
  }
  m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({0, 7})], false));
  m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({7, 7})], false));
  m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({1, 7})], false));
  m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({6, 7})], false));
  m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({2, 7})], false));
  m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({5, 7})], false));
  m_WhitePieces.insert(std::make_shared<Queen>(&board[posToIndex({3, 7})], false));
  m_WhitePieces.insert(std::make_shared<King>(&board[posToIndex({4, 7})], false));
  m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({0, 0})], true));
  m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({7, 0})], true));
  m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({1, 0})], true));
  m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({6, 0})], true));
  m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({2, 0})], true));
  m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({5, 0})], true));
  m_BlackPieces.insert(std::make_shared<Queen>(&board[posToIndex({3, 0})], true));
  m_BlackPieces.insert(std::make_shared<King>(&board[posToIndex({4, 0})], true));

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

Square *ChessGame::getSquareAtPosition(std::array<Square, 64> &board, int x, int y) {
  return &board[posToIndex({x, y})];
}

void ChessGame::handleMouseClick(SDL_Event* event) {
  // Only handle click events for current player's turn
  int mouseX = static_cast<int>(event->button.x), mouseY = static_cast<int>(event->button.y);
  LOG_PRINTF("Mouse clicked at: (%d, %d)\n", mouseX, mouseY);
  int posX = mouseX / SQUARE_SIZE;
  int posY = mouseY / SQUARE_SIZE;

  if (!m_SelectedSquare) {
    selectSource(posX, posY);
  } else {
    selectDestination(posX, posY);
  }
}

void ChessGame::selectSource(int x, int y) {
  Square* clickedSquare = getSquareAtPosition(m_Board, x, y);
  if (!clickedSquare) {
    std::cerr << "Clicked outside of board boundaries.\n";
    throw std::runtime_error("Clicked outside of board boundaries");
  }
  if (clickedSquare->occupyingPiece == nullptr) {
    LOG_COUT("You must select one of your own pieces");
    return;
  }
  if (clickedSquare->occupyingPiece->isBlack() && !m_PlayerIsBlack ||
      !clickedSquare->occupyingPiece->isBlack() && m_PlayerIsBlack) {
    LOG_COUT("You clicked on an opponent's piece");
    return;
  }
  m_SelectedSquare = clickedSquare;
  clickedSquare->isHighlighted = true;

  // TODO: Highlight possible moves, implement taking pieces
  std::vector<Position> possibleMoves = clickedSquare->occupyingPiece->getPossibleMoves(m_Board);
  for (const Position &pos : possibleMoves) {
    Square* targetSquare = getSquareAtPosition(m_Board, pos.x, pos.y);
    if (!targetSquare) {
      std::cerr << "No target square found\n";
      throw std::runtime_error("No target square found\n");
    }
    targetSquare->isHighlighted = true;
  }
}

void ChessGame::selectDestination(int x, int y) {
  Square* clickedSquare = getSquareAtPosition(m_Board, x, y);
  if (!clickedSquare) {
    std::cerr << "Clicked outside of board boundaries.\n";
    throw std::runtime_error("Clicked outside of board boundaries");
    return;
  }

  // Deselect piece after move attempt
  if (clickedSquare->isHighlighted && clickedSquare != m_SelectedSquare) {
    LOG_COUT("Moving piece to new square.");
    processMove(m_SelectedSquare->x, m_SelectedSquare->y, clickedSquare->x, clickedSquare->y);
  }
  unselectAllSquares();
  m_SelectedSquare = nullptr;
  return;
}

void ChessGame::processMove(int srcX, int srcY, int dstX, int dstY) {
  // Generate next board's game state
  // TODO: Validate the next board
  std::array<Square, 64> nextBoard = m_Board;
  Square* srcSquare = getSquareAtPosition(nextBoard, srcX, srcY);
  Square* dstSquare = getSquareAtPosition(nextBoard, dstX, dstY);

  std::shared_ptr<Piece> srcPiece = srcSquare->occupyingPiece;
  std::shared_ptr<Piece> dstPiece = dstSquare->occupyingPiece;

  bool isValidMove = true; // TODO: Replace this with functionality that validates nextBoard

  LOG_PRINTF("Moving from (%d, %d) (%p) to (%d, %d) (%p)\n", srcX, srcY, srcSquare, dstX, dstY, dstSquare);
  // Perform the move if it is valid
  if (isValidMove)
  {
    srcSquare = getSquareAtPosition(m_Board, srcX, srcY);
    dstSquare = getSquareAtPosition(m_Board, dstX, dstY);
    if (srcPiece) {
      srcPiece->setSquare(getSquareAtPosition(m_Board, dstX, dstY));
      dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);
    }
    if (dstPiece) {
      dstPiece->setIsAlive(false);
    }
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