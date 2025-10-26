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

  void ChessGame::setupInitialPieces(std::array<Square, 64>& board) {
    // Generate pieces
    for (int i = 0; i < 8; i++) {
      m_WhitePieces.insert(std::make_shared<Pawn>(&board[posToIndex({ i, 6 })], false));
      m_BlackPieces.insert(std::make_shared<Pawn>(&board[posToIndex({ i, 1 })], true));
    }
    m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({ 0, 7 })], false));
    m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({ 7, 7 })], false));
    m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({ 1, 7 })], false));
    m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({ 6, 7 })], false));
    m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 2, 7 })], false));
    m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 5, 7 })], false));
    m_WhitePieces.insert(std::make_shared<Queen>(&board[posToIndex({ 3, 7 })], false));
    m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({ 0, 0 })], true));
    m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({ 7, 0 })], true));
    m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({ 1, 0 })], true));
    m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({ 6, 0 })], true));
    m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 2, 0 })], true));
    m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 5, 0 })], true));
    m_BlackPieces.insert(std::make_shared<Queen>(&board[posToIndex({ 3, 0 })], true));

    m_WhiteKing = std::make_shared<King>(&board[posToIndex({ 4, 7 })], false);
    m_BlackKing = std::make_shared<King>(&board[posToIndex({ 4, 0 })], true);

    m_WhitePieces.insert(m_WhiteKing);
    m_BlackPieces.insert(m_BlackKing);

    for (const auto& piece : m_WhitePieces) {
      int index = posToIndex({ piece->getPosition().x, piece->getPosition().y });
      m_Board[index].occupyingPiece = piece;
    }

    for (const auto& piece : m_BlackPieces) {
      int index = posToIndex({ piece->getPosition().x, piece->getPosition().y });
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

  void ChessGame::handleMouseClick(SDL_Event* event) {
    // Only handle click events for current player's turn
    Position pos = m_RenderHandler.mouseToPosition(event);

    if (!m_SelectedSquare) {
      selectSource(pos.x, pos.y);
    }
    else {
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

    m_MovesForSelected = clickedSquare->occupyingPiece->getPossibleMoves(m_Board, m_ActionHistory);

    for (const Move& move : m_MovesForSelected) {
      if (!isValidMove(m_SelectedSquare->occupyingPiece, move)) {
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
    auto selectedValidMove = std::find_if(
      m_MovesForSelected.begin(), m_MovesForSelected.end(),
      [&](const Move& move) { return move.dst.x == x && move.dst.y == y; });
    if (selectedValidMove != m_MovesForSelected.end()) {
      processMove(m_SelectedSquare->occupyingPiece, *selectedValidMove);
    }
    unselectAllSquares();
    m_SelectedSquare = nullptr;
    return;
  }

  void ChessGame::processMove(const std::shared_ptr<Piece>& piece, const Move& move) {
    // This assumes that the move is already validated and generates the next state
    // and also checks if the next state would put the player in checkmate.
    Square* srcSquare = getSquareAtPosition(m_Board, piece->getPosition().x, piece->getPosition().y);
    Square* dstSquare = getSquareAtPosition(m_Board, move.dst.x, move.dst.y);

    m_ActionHistory.push_back({ piece, move });

    if (move.capturedPiece) {
      // Captured piece is not always the destination square in moves like en passante
      Position capturedPos = move.capturedPiece->getPosition();
      Square* capturedSquare = getSquareAtPosition(m_Board, capturedPos.x, capturedPos.y);
      capturedSquare->occupyingPiece = nullptr;

      move.capturedPiece->setIsAlive(false);
      m_RenderHandler.capturePiece(move.capturedPiece);
    }

    piece->performMove(m_Board, move);
    dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);



    // Switch turn
    m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
    // Check if current color is in checkmate
    bool currentColorCheckmate = true;
    for (const std::shared_ptr<Piece> piece : m_CurrentTurnColor == BLACK ? m_BlackPieces : m_WhitePieces) {
      std::vector<Move> possibleMoves = piece->getPossibleMoves(m_Board, m_ActionHistory);
      for (const Move& move : possibleMoves) {
        if (isValidMove(piece, move)) {
          currentColorCheckmate = false;
          goto CHECKMATE_CHECK_EXIT;
        };
      }
    }
  CHECKMATE_CHECK_EXIT:
    if (currentColorCheckmate) {
      if (m_CurrentTurnColor == BLACK) {
        MessageBox(NULL, L"Black has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
      }
      if (m_CurrentTurnColor == WHITE) {
        MessageBox(NULL, L"White has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
      }
    }

    // For now, switch players, until 2p capabilities or bot capabilities added
    m_CurrentPlayerColor = m_CurrentPlayerColor == BLACK ? WHITE : BLACK;
  }

  bool ChessGame::isValidMove(const std::shared_ptr<Piece>& piece, const Move& move) {
    // Create copy of current board and check the board state if the selected piece was at pos
    std::array<Square, 64> boardCopy = m_Board;

    // Temporarily kill off the captured piece
    if (move.capturedPiece) {
      move.capturedPiece->setIsAlive(false);
    }

    const Position& piecePos = piece->getPosition();

    Square* selectedSquareCopy = getSquareAtPosition(boardCopy, piecePos.x, piecePos.y);
    boardCopy[posToIndex(move.dst)].occupyingPiece = std::move(selectedSquareCopy->occupyingPiece);

    bool isValidMove = !isKingInCheck(boardCopy, m_CurrentTurnColor);

    if (move.capturedPiece) {
      move.capturedPiece->setIsAlive(true);
    }

    return isValidMove;
  }

  bool ChessGame::isKingInCheck(std::array<Square, 64>& board, PlayerColor Color) {

    const std::set<std::shared_ptr<Piece>>& opponentsPieces = Color == WHITE ? m_BlackPieces : m_WhitePieces;
    const std::shared_ptr<Piece>& myKing = Color == WHITE ? m_WhiteKing : m_BlackKing;

    for (const auto& opponentPiece : opponentsPieces) {
      std::vector<Move> possibleMoves = opponentPiece->getPossibleMoves(board, m_ActionHistory);
      auto it = std::find_if(
        possibleMoves.begin(), possibleMoves.end(),
        [&](const Move& move) { return move.capturedPiece == myKing; });
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
    for (Square& square : m_Board) {
      square.isHighlighted = false;
    }
    m_MovesForSelected.clear();
  }
} // namespace chess_client