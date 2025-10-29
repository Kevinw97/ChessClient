#include "chess_game.h"
#include "sdl_render_handler.h"
#include "sdl_audio_handler.h"
#include <iostream>
#include <thread>

namespace chess_client {


  ChessGame::ChessGame()
    : m_RenderHandler("Chess Game", BOARD_SIZE, BOARD_SIZE)
    , m_Board{} {
    m_RenderHandler.generateInitialBoard(m_Board);
    setupInitialPieces(m_Board);
  }

  void ChessGame::setupInitialPieces(std::array<Square, 64>& board) {
    // Generate pieces
    for (int i = 0; i < 8; i++) {
      m_WhitePieces.insert(std::make_shared<Pawn>(&board[posToIndex({ i, 6 })], WHITE));
      m_BlackPieces.insert(std::make_shared<Pawn>(&board[posToIndex({ i, 1 })], BLACK));
    }
    m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({ 0, 7 })], WHITE));
    m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({ 7, 7 })], WHITE));
    m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({ 1, 7 })], WHITE));
    m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({ 6, 7 })], WHITE));
    m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 2, 7 })], WHITE));
    m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 5, 7 })], WHITE));
    m_WhitePieces.insert(std::make_shared<Queen>(&board[posToIndex({ 3, 7 })], WHITE));
    m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({ 0, 0 })], BLACK));
    m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({ 7, 0 })], BLACK));
    m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({ 1, 0 })], BLACK));
    m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({ 6, 0 })], BLACK));
    m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 2, 0 })], BLACK));
    m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({ 5, 0 })], BLACK));
    m_BlackPieces.insert(std::make_shared<Queen>(&board[posToIndex({ 3, 0 })], BLACK));

    m_WhiteKing = std::make_shared<King>(&board[posToIndex({ 4, 7 })], WHITE);
    m_BlackKing = std::make_shared<King>(&board[posToIndex({ 4, 0 })], BLACK);

    m_WhitePieces.insert(m_WhiteKing);
    m_BlackPieces.insert(m_BlackKing);

    for (const auto& piece : m_WhitePieces) {
      int index = posToIndex(piece->getSquare()->pos);
      m_Board[index].occupyingPiece = piece;
    }

    for (const auto& piece : m_BlackPieces) {
      int index = posToIndex(piece->getSquare()->pos);
      m_Board[index].occupyingPiece = piece;
    }
  }

  void ChessGame::gameSetup() {
    std::cout << "Choose singleplayer[s] or online[m]: ";
    std::string userInput;
    while (std::getline(std::cin, userInput)) {
      std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower);
      if (userInput == "s") {
        m_Online = false;
        return;
      }
      else if (userInput == "m") {
        m_Online = true;;
        break;
      }
      else {
        std::cout << "Invalid input. Choose singleplayer[s] or online[m]: ";
      }
    }
    userInput.clear();
    std::cout << "Enter server address: ";
    while (std::getline(std::cin, userInput)) {
      struct sockaddr_in sa;
      int result = inet_pton(AF_INET, userInput.c_str(), &(sa.sin_addr));
      if (result) {
        m_ServerIp = userInput;
        break;
      }
      else {
        std::cout << "Invalid input. Choose singleplayer or multiplayer: ";
      }
    }
  }

  void ChessGame::gameLoop() {
    SDL_Event event;
    while (m_InProgress && SDL_WaitEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT: {
        m_Running = false;
        if (m_Online) {
          // Cleanup connection
          closesocket(m_ClientSocket);
          WSACleanup();
          m_ListenerThread.join();
        }
        return;
      }
      case SDL_EVENT_MOUSE_BUTTON_UP: {
        if (!isCurrentPlayersTurn()) {
          break;
        }
        handleMouseClick(&event);
        m_RenderHandler.drawChessBoard(m_Board);
        m_RenderHandler.drawCapturedPieces();
        break;
      }
      case SDL_EVENT_KEY_UP: {
        if (event.key.key == SDLK_F4) {
          m_InProgress = false;
        }
        if (event.key.key == SDLK_LEFT) {
          undoMove();
          m_RenderHandler.drawChessBoard(m_Board);
          m_RenderHandler.drawCapturedPieces();
        }
        break;
      }
      }
    }
  }

  void ChessGame::run() {
    gameSetup();
    if (m_Online) {
      setupOnlineClient();
    }

    m_RenderHandler.init();
    m_AudioHandler.init();

    m_RenderHandler.drawChessBoard(m_Board);

    while (m_Running) {
      if (m_InProgress) {
        gameLoop();
      }
      else {
        LOG_COUT("Press enter to reset game...");
        std::cin;
        LOG_PRINTF("Resetting game in 5 seconds\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LOG_PRINTF("Resetting game in 4 seconds\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LOG_PRINTF("Resetting game in 3 seconds\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LOG_PRINTF("Resetting game in 2 seconds\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LOG_PRINTF("Resetting game in 1 seconds\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        resetGame();
      }
    }
  }

  void ChessGame::setupOnlineClient() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
      LOG_COUT("WSAStartup failed: " << result);;
      return;
    }

    m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_ClientSocket == INVALID_SOCKET) {
      LOG_COUT("Socket creation failed: " << WSAGetLastError());
      WSACleanup();
      return;
    }

    // Setup server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12312);
    inet_pton(AF_INET, m_ServerIp.c_str(), &serverAddr.sin_addr);

    // Connect to server
    result = connect(m_ClientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
      LOG_COUT("Connection failed: " << WSAGetLastError());
      closesocket(m_ClientSocket);
      WSACleanup();
      return;
    }

    std::cout << "Connected to server!" << std::endl;

    std::cout << "Waiting for an opponent.." << std::endl;
    // Receive match setup information from server
    char buffer[512] = { 0 };
    int bytesReceived = recv(m_ClientSocket, buffer, 512, 0);
    if (bytesReceived <= 0) {
      LOG_COUT("Connection closed by server or recv failed");
      closesocket(m_ClientSocket);
      WSACleanup();
    }
    else {
      m_PlayerColor = static_cast<PieceColor>(buffer[0]);
    }

    m_ListenerThread = std::thread(&ChessGame::listenLoop, this);
  }

  void ChessGame::listenLoop() {
    while (m_Running) {  // Check running flag
      // Receive response
      std::array<char, 512> buffer = { 0 };
      int bytesReceived = recv(m_ClientSocket, buffer.data(), 512, 0);
      if (bytesReceived > 0) {
        std::cout << "Received " << bytesReceived << " bytes:" << std::endl;

        // Print byte data in hexadecimal format
        for (int i = 0; i < bytesReceived; i++) {
          printf("%02X ", static_cast<unsigned char>(buffer[i]));
          if ((i + 1) % 16 == 0) {  // New line every 16 bytes
            std::cout << std::endl;
          }
        }
        std::cout << std::endl;
        if (!isCurrentPlayersTurn()) {
          Move move = decodeMove(buffer);
          if (std::shared_ptr<Piece> piece = m_Board[posToIndex(move.src)].occupyingPiece) {
            processMove(piece, move);
            m_RenderHandler.drawChessBoard(m_Board);
            m_RenderHandler.drawCapturedPieces();
          }
        }
      }
      else if (bytesReceived == 0) {
        std::cout << "Connection closed by server" << std::endl;
        break;
      }
      else {
        std::cout << "Recv failed: " << WSAGetLastError() << std::endl;
        break;
      }
    }
  }

  void ChessGame::handleMouseClick(SDL_Event* event) {
    // Only handle click events for current player's turn
    Position pos = m_RenderHandler.mouseToPosition(event);
    if (isValidPosition(pos)) {
      if (!m_SelectedSquare) {
        selectSource(pos.x, pos.y);
      }
      else {
        selectDestination(pos.x, pos.y);
      }
    }
  }

  void ChessGame::selectSource(int x, int y) {
    Square* clickedSquare = getSquareAtPosition(m_Board, { x, y });
    if (!clickedSquare) {
      LOG_COUT("Clicked outside of the Chess board");
      return;
    }
    if (clickedSquare->occupyingPiece == nullptr) {
      LOG_COUT("You must select one of your own pieces");
      return;
    }
    if (clickedSquare->occupyingPiece->getColor() != m_CurrentTurnColor) {
      LOG_COUT("You clicked on an opponent's piece");
      return;
    }
    m_SelectedSquare = clickedSquare;
    clickedSquare->isHighlighted = true;

    auto possibleMoves = clickedSquare->occupyingPiece->getPossibleMoves(m_Board, m_ActionHistory);
    for (const Move& move : possibleMoves) {
      if (!isValidMove(m_SelectedSquare->occupyingPiece, move)) {
        continue;
      };
      Square* targetSquare = getSquareAtPosition(m_Board, move.dst);
      if (!targetSquare) {
        std::cerr << "No target square found\n";
        throw std::runtime_error("No target square found\n");
      }
      targetSquare->isHighlighted = true;
      m_MovesForSelected.push_back(move);
    }
  }

  void ChessGame::selectDestination(int x, int y) {
    // Deselect piece after move attempt
    auto selectedValidMove = std::find_if(
      m_MovesForSelected.begin(), m_MovesForSelected.end(),
      [&](Move& move) { return move.dst.x == x && move.dst.y == y; });
    if (selectedValidMove != m_MovesForSelected.end()) {
      auto pawn = std::dynamic_pointer_cast<Pawn>(m_SelectedSquare->occupyingPiece);
      if (pawn && pawn->canPromote(*selectedValidMove)) {
        std::cout << "Pawn promotion. Choose a piece (rook, bishop, knight, queen): ";
        std::string pieceInput;
        PieceType selectedType = QUEEN; // Default to queen
        while (std::getline(std::cin, pieceInput)) {
          std::transform(pieceInput.begin(), pieceInput.end(), pieceInput.begin(), ::tolower);
          if (pieceInput == "rook") {
            selectedType = ROOK;
            break;
          }
          else if (pieceInput == "bishop") {
            selectedType = BISHOP;
            break;
          }
          else if (pieceInput == "knight") {
            selectedType = KNIGHT;
            break;
          }
          else if (pieceInput == "queen") {
            selectedType = QUEEN;
            break;
          }
          else {
            std::cout << "Invalid input. Choose rook, bishop, knight, or queen: ";
          }
        }
        selectedValidMove->promoteType = selectedType;
      }
      processMove(m_SelectedSquare->occupyingPiece, *selectedValidMove);
    }
    unselectAllSquares();
    
    return;
  }

  void ChessGame::processMove(const std::shared_ptr<Piece>& piece, Move& move) {
    // This assumes that the move is already validated and generates the next state
    // and also checks if the next state would put the player in checkmate.

    Square* srcSquare = piece->getSquare();
    Square* dstSquare = getSquareAtPosition(m_Board, move.dst);

    if (isCurrentPlayersTurn()) {
      writeBoard();
    }

    if (move.capturedPiece) {
      // Captured piece is not always the destination square in moves like en passante
      Square* capturedSquare = move.capturedPiece->getSquare();
      capturedSquare->occupyingPiece = nullptr;

      move.capturedPiece->setIsAlive(false);
      m_RenderHandler.capturePiece(move.capturedPiece);
    }

    piece->performMove(m_Board, move);
    dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);

    // Castling
    if (move.castlingRook && isValidPosition(move.castlingRookDst)) {
      Square* rookSrcSquare = move.castlingRook->getSquare();
      Square* rookDstSquare = getSquareAtPosition(m_Board, move.castlingRookDst);

      move.castlingRook->performMove(m_Board, move);
      rookDstSquare->occupyingPiece = std::move(rookSrcSquare->occupyingPiece);
    }

    if (!m_Online) {
      // Push performed action to stack
      m_ActionHistory.push_back({ dstSquare->occupyingPiece, move });
    }

    // Check if current color is in checkmate
    bool currentColorCheckmate = true;
    for (const std::shared_ptr<Piece> &piece : m_CurrentTurnColor == BLACK ? m_BlackPieces : m_WhitePieces) {
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
        m_InProgress = false;
      }
      if (m_CurrentTurnColor == WHITE) {
        MessageBox(NULL, L"White has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
        m_InProgress = false;
      }
    }

    if (m_Online && isCurrentPlayersTurn()) {
      writeMove(piece, move);
      sendCommand();
    }

    if (!m_Online) {
      // Swap player color for single player testing
      m_PlayerColor = m_PlayerColor == BLACK ? WHITE : BLACK;
    }

    // Switch turn
    m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;

    if (move.capturedPiece) {
      m_AudioHandler.playCaptureSound();
    }
    else {
      m_AudioHandler.playMoveSound();
    }
  }

  bool ChessGame::isValidMove(const std::shared_ptr<Piece>& piece, const Move& move) {
    // Create copy of current board and check the board state if the selected piece was at pos
    std::array<Square, 64> boardCopy = m_Board;

    // Temporarily kill off the captured piece
    if (move.capturedPiece) {
      move.capturedPiece->setIsAlive(false);
    }

    const Position &piecePos = piece->getSquare()->pos;
    Square *selectedSquareCopy = getSquareAtPosition(boardCopy, piecePos);
    boardCopy[posToIndex(move.dst)].occupyingPiece = std::move(selectedSquareCopy->occupyingPiece);

    // If castling, move rook to the castling position as well
    if (move.castlingRook && isValidPosition(move.castlingRookDst)) {
      Square *rookSquareCopy = getSquareAtPosition(boardCopy, piecePos);
      boardCopy[posToIndex(move.castlingRookDst)].occupyingPiece = std::move(rookSquareCopy->occupyingPiece);
    }

    bool isValidMove = !isKingInCheck(boardCopy, m_CurrentTurnColor);

    if (move.capturedPiece) {
      move.capturedPiece->setIsAlive(true);
    }

    return isValidMove;
  }

  bool ChessGame::isKingInCheck(std::array<Square, 64>& board, PieceColor Color) {

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
    return m_CurrentTurnColor == m_PlayerColor;
  }

  void ChessGame::unselectAllSquares() {
    for (Square& square : m_Board) {
      square.isHighlighted = false;
    }
    m_SelectedSquare = nullptr;
    m_MovesForSelected.clear();
  }

  void ChessGame::undoMove() {
    if (m_ActionHistory.empty()) {
      return;
    }
    Action previousAction = m_ActionHistory.back();
    Move move = previousAction.move;
    std::shared_ptr<Piece> piece = previousAction.piece;
    m_ActionHistory.pop_back();

    std::swap(move.src, move.dst);
    std::swap(move.castlingRookSrc, move.castlingRookDst);

    Square* srcSquare = piece->getSquare();
    Square* dstSquare = getSquareAtPosition(m_Board, move.dst);

    piece->performMove(m_Board, move);
    dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);

    if (move.capturedPiece) {
      // Revive piece that was captured, it should still reference
      // the square that it was captured from
      Square* capturedSquare = move.capturedPiece->getSquare();
      capturedSquare->occupyingPiece = move.capturedPiece;

      move.capturedPiece->setIsAlive(true);
      m_RenderHandler.undoCapture(move.capturedPiece->getColor());
    }

    // Undo castling
    if (move.castlingRook && isValidPosition(move.castlingRookDst)) {
      Square* rookSrcSquare = move.castlingRook->getSquare();
      Square* rookDstSquare = getSquareAtPosition(m_Board, move.castlingRookDst);

      move.castlingRook->performMove(m_Board, move);
      rookDstSquare->occupyingPiece = std::move(rookSrcSquare->occupyingPiece);
    }

    // Undo promote
    if (move.promoteType) {
      auto pawn = std::dynamic_pointer_cast<Pawn>(piece);
      if (pawn) {
        pawn->undoPromote();
      }
    }

    // Switch turn
    m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;

    // For now, switch players, until 2p capabilities or bot capabilities added
    m_PlayerColor = m_PlayerColor == BLACK ? WHITE : BLACK;
  }

  void ChessGame::writeBoard() {
    // Encode the curent board state to send
    for (auto square = m_Board.begin(); square != m_Board.end(); square++) {
      std::shared_ptr<Piece> piece = square->occupyingPiece;
      if (piece) {
        m_DataBuffer.push_back(piece->getColor());
        m_DataBuffer.push_back(piece->getType());
      }
      else {
        m_DataBuffer.push_back(0);
        m_DataBuffer.push_back(0);
      }
    }
  }

  void ChessGame::writeMove(const std::shared_ptr<Piece>& piece, const Move& move) {
    NetworkMove networkMove;
    networkMove.src = move.src;
    networkMove.dst = move.dst;
    networkMove.capturedPiece = getPieceKey(move.capturedPiece);
    networkMove.castlingRookSrc = move.castlingRookSrc;
    networkMove.castlingRookDst = move.castlingRookDst;
    networkMove.castlingRook = getPieceKey(move.castlingRook);
    networkMove.promoteType = move.promoteType;
    networkMove.firstMove = move.firstMove;

    const unsigned char* moveBytes = reinterpret_cast<const unsigned char*>(&networkMove);
    m_DataBuffer.insert(m_DataBuffer.end(), moveBytes, moveBytes + sizeof(NetworkMove));
  }

  void ChessGame::sendCommand() {
    send(m_ClientSocket, reinterpret_cast<const char*>(m_DataBuffer.data()), static_cast<int>(m_DataBuffer.size()), 0);
    m_DataBuffer.clear();
  }
  
  Move ChessGame::decodeMove(std::array<char, 512> data) {
    for (int i = 0; i < 64; i++) {
      // Do some validation with the board
    }

    // Cast the data starting at position 128 to a NetworkMove pointer
    const NetworkMove* networkMove = reinterpret_cast<const NetworkMove*>(&data[128]);

    // Decode into a Move struct
    Move move;
    move.src = networkMove->src;
    move.dst = networkMove->dst;
    move.castlingRookSrc = networkMove->castlingRookSrc;
    move.castlingRookDst = networkMove->castlingRookDst;
    move.promoteType = networkMove->promoteType;
    move.firstMove = networkMove->firstMove;

    // Decode piece keys back to shared_ptr<Piece>
    if (networkMove->capturedPiece) {
      unsigned char key = networkMove->capturedPiece;
      move.capturedPiece = m_Pieces[key];
    }

    if (networkMove->castlingRook) {
      unsigned char key = networkMove->castlingRook;
      move.castlingRook = m_Pieces[key];
    }

    return move;
  }

  unsigned char ChessGame::getPieceKey(const std::shared_ptr<Piece>& piece) {
    if (piece) {
      return static_cast<unsigned char>(piece->getInitialPosition().x) << 4 | static_cast<unsigned char>(piece->getInitialPosition().y) | 0x80;
    }
    return 0;
  }

  void ChessGame::resetGame() {
    for (Square &square : m_Board) {
      square.occupyingPiece = nullptr;
    }
    for (auto& piece : m_WhitePieces) {
      piece->resetPiece(m_Board);
      Square* initialSquare = getSquareAtPosition(m_Board, piece->getInitialPosition());
      initialSquare->occupyingPiece = piece;
    }
    for (auto& piece : m_BlackPieces) {
      piece->resetPiece(m_Board);
      Square* initialSquare = getSquareAtPosition(m_Board, piece->getInitialPosition());
      initialSquare->occupyingPiece = piece;
    }
    unselectAllSquares();
    m_ActionHistory.clear();
    m_InProgress = true;
    m_CurrentTurnColor = WHITE;
    // Temporary until online functionality added
    m_PlayerColor = WHITE;
    // ------------------------------------------
    m_RenderHandler.clearCapturedPieces();
    m_RenderHandler.drawChessBoard(m_Board);
  }
} // namespace chess_client