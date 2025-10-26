#include "chess_game.h"
#include "render_handler.h"
#include <iostream>
#include <thread>

namespace chess_client {


ChessGame::ChessGame()
    : m_RenderHandler("Chess Game", BOARD_SIZE, BOARD_SIZE)
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
  m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({0, 0})], true));
  m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({7, 0})], true));
  m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({1, 0})], true));
  m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({6, 0})], true));
  m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({2, 0})], true));
  m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({5, 0})], true));
  m_BlackPieces.insert(std::make_shared<Queen>(&board[posToIndex({3, 0})], true));

  m_WhiteKing = std::make_shared<King>(&board[posToIndex({4, 7})], false);
  m_BlackKing = std::make_shared<King>(&board[posToIndex({4, 0})], true);

  m_WhitePieces.insert(m_WhiteKing);
  m_BlackPieces.insert(m_BlackKing);

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
      m_RenderHandler.drawChessBoard(m_Board);
      m_RenderHandler.drawCapturedPieces();
    }
    }
  }
}

void ChessGame::run() {
  gameLoop(); 
}

Square *ChessGame::getSquareAtPosition(std::array<Square, 64> &board, int x, int y) {
  int index = posToIndex({x, y});
  if (index == -1) {
    return nullptr;
  }
  return &board[index];
}

void ChessGame::handleMouseClick(SDL_Event* event) {
  // Only handle click events for current player's turn
  Position pos = m_RenderHandler.mouseToPosition(event);

  if (!m_SelectedSquare) {
    selectSource(pos.x, pos.y);
  } else {
    selectDestination(pos.x, pos.y);
  }
}

void ChessGame::selectSource(int x, int y) {
  Square* clickedSquare = getSquareAtPosition(m_Board, x, y);
  if (!clickedSquare) {
    LOG_COUT("Clicked outside of the Chess board");
    return;
  }
  if (clickedSquare->occupyingPiece == nullptr) {
    LOG_COUT("You must select one of your own pieces");
    return;
  }
  if (clickedSquare->occupyingPiece->isBlack() && m_CurrentTurnColor == WHITE ||
      !clickedSquare->occupyingPiece->isBlack() && m_CurrentTurnColor == BLACK) {
    LOG_COUT("You clicked on an opponent's piece");
    return;
  }
  m_SelectedSquare = clickedSquare;
  clickedSquare->isHighlighted = true;

  std::vector<Move> possibleMoves = clickedSquare->occupyingPiece->getPossibleMoves(m_Board);

  for (const Move &move : possibleMoves) {
    // TODO: For each possible move, we need to check whether it would put the current player's King in check.
    if (!isValidMove(move)) {
      continue;
    };
    Square* targetSquare = getSquareAtPosition(m_Board, move.dst.x, move.dst.y);
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
      m_RenderHandler.capturePiece(dstPiece);
    }
  }

  // Switch turn
  m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
  // For now, switch players, until 2p capabilities or bot capabilities added
  m_CurrentPlayerColor = m_CurrentPlayerColor == BLACK ? WHITE : BLACK;
}

bool ChessGame::isValidMove(const Move &move) {
  // A square with an occupying piece must be selected
  if (!m_SelectedSquare || !m_SelectedSquare->occupyingPiece) {
    return false;
  }

  // Create copy of current board and check the board state if the selected piece was at pos
  std::array<Square, 64> boardCopy = m_Board;
  
  // Temporarily kill off the captured piece
  if (move.capturedPiece) {
    move.capturedPiece->setIsAlive(false);
  }

  Square *selectedSquareCopy = getSquareAtPosition(boardCopy, m_SelectedSquare->x, m_SelectedSquare->y);
  boardCopy[posToIndex(move.dst)].occupyingPiece = std::move(selectedSquareCopy->occupyingPiece);

  bool isValidMove = !isKingInCheck(boardCopy, m_CurrentTurnColor);

  if (move.capturedPiece) {
    move.capturedPiece->setIsAlive(true);
  }

  return isValidMove;
}

bool ChessGame::isKingInCheck(std::array<Square, 64> &board, PlayerColor Color) {

  const std::set<std::shared_ptr<Piece>> &opponentsPieces = Color == WHITE ? m_BlackPieces : m_WhitePieces;
  const std::shared_ptr<Piece> &myKing = Color == WHITE ? m_WhiteKing : m_BlackKing;

  for (const auto &opponentPiece : opponentsPieces) {
    std::vector<Move> possibleMoves = opponentPiece->getPossibleMoves(board);
    auto it = std::find_if(
        possibleMoves.begin(), possibleMoves.end(),
        [&](const Move &move) { return move.capturedPiece == myKing; });
    if (it != possibleMoves.end()) {
      return true;
    }
  }
  return false;
}

bool ChessGame::isCurrentPlayersTurn() {
  return m_CurrentPlayerColor == m_CurrentTurnColor;
}

void ChessGame::unselectAllSquares() {
  for (Square &square : m_Board) {
    square.isHighlighted = false;
  }
}
} // namespace chess_client