#include "chess_game.h"
#include "sdl_render_handler.h"
#include "sdl_audio_handler.h"
#include <iostream>
#include <thread>

namespace chess_client {


  ChessGame::ChessGame()
    : m_RenderHandler("Chess Game", BOARD_SIZE, BOARD_SIZE)
    , m_Board{} 
    , m_ClientSocket(INVALID_SOCKET) {
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
      m_Pieces.emplace(getPieceKey(piece), piece);
    }

    for (const auto& piece : m_BlackPieces) {
      int index = posToIndex(piece->getSquare()->pos);
      m_Board[index].occupyingPiece = piece;
      m_Pieces.emplace(getPieceKey(piece), piece);
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
        m_Online = true;
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
          if (m_ClientSocket != INVALID_SOCKET) {
            closesocket(m_ClientSocket);
            WSACleanup();
          }
          if (m_ListenerThread.joinable()) {
            m_ListenerThread.join();
          }
        }
        return;
      }
      case SDL_EVENT_MOUSE_BUTTON_UP: {
        if (!isCurrentPlayersTurn()) {
          break;
        }
        handleMouseClick(&event);
        break;
      }
      case SDL_EVENT_KEY_UP: {
        if (event.key.key == SDLK_F4) {
          m_InProgress = false;
        }
        if (event.key.key == SDLK_LEFT) {
          //undoMove();
          //m_RenderHandler.drawChessBoard(m_Board);
          //m_RenderHandler.drawCapturedPieces();
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
        if (!m_Online) {
          resetGame();
        }
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

    if (isCurrentPlayersTurn()) {
      m_State = WAITING_FOR_PLAYER;
    }

    m_ListenerThread = std::thread(&ChessGame::listenLoop, this);
  }

  void ChessGame::listenLoop() {
    while (m_Running) {  // Check running flag
      // Receive response
      std::array<char, 4096> buffer = { 0 };
      int bytesReceived = recv(m_ClientSocket, buffer.data(), 512, 0);
      if (bytesReceived > 0) {
        std::cout << "Received " << bytesReceived << " bytes:" << std::endl;
        int i = 0;
        unsigned char *response = reinterpret_cast<unsigned char*>(buffer.data());
        if (response[i] == 0x55) {
          LOG_COUT("SDKLMFKDLSFMDF");
          i++;

          // Get piece
          if (i + 1 > bytesReceived) {
            std::cerr << "Didn't receive the piece\n";
            throw std::runtime_error("Didn't receive the piece\n");
          }
          unsigned char pieceKey = response[i];
          std::shared_ptr<Piece> piece = getPiece(pieceKey);
          if (!piece) {
            std::cerr << "No piece found\n";
            throw std::runtime_error("No piece found\n");
          }
          i++;

          // Get and decode network move
          if (i + sizeof(NetworkMove) > bytesReceived) {
            std::cerr << "Didn't receive entire network move\n";
            throw std::runtime_error("Didn't receive entire network move, partial read or something else?\n");
          }
          Move move = decodeMove(&response[i]);
          i += sizeof(NetworkMove);

          // Process the move, and validate that the board matches the state sent by server
          processMove(piece, move);

          if (i + 64 > bytesReceived) {
            std::cerr << "Didn't receive entire board\n";
            throw std::runtime_error("Didn't receive entire board, partial read or something else?\n");
          }
          if (!validateBoard(&response[i])) {
            std::cerr << "Board doesn't match\n";
            throw std::runtime_error("Board doesn't match\n");
          }
          i += 64;

          if (i + 1 > bytesReceived) {
            std::cerr << "Didn't receive turn\n";
            throw std::runtime_error("Didn't receive turn\n");
          }
          m_CurrentTurnColor = static_cast<PieceColor>(response[i]);
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

    m_RenderHandler.drawChessBoard(m_Board);
    m_RenderHandler.drawCapturedPieces();
  }

  void ChessGame::selectDestination(int x, int y) {
    // Deselect piece after move attempt
    if (!m_SelectedSquare || !m_SelectedSquare->occupyingPiece) {
      return;
    }

    auto selectedValidMove = std::find_if(m_MovesForSelected.begin(), m_MovesForSelected.end(),
      [&](Move& move) { return move.dst.x == x && move.dst.y == y; });

    if (selectedValidMove == m_MovesForSelected.end()) {
      unselectAllSquares();
      m_RenderHandler.drawChessBoard(m_Board);
      m_RenderHandler.drawCapturedPieces();
      return;
    }

    Move move = *selectedValidMove;
    std::shared_ptr<Piece> piece = m_SelectedSquare->occupyingPiece;
    unselectAllSquares();

    // If move results in promotion, prompt for choice
    choosePawnPromotion(piece, move);

    if (m_Online) {
      sendMove(piece, move);
    }
    else {
      processMove(piece, move);
    }

    return;
  }

  void ChessGame::choosePawnPromotion(const std::shared_ptr<Piece>& piece, Move& move) {
    auto pawn = std::dynamic_pointer_cast<Pawn>(piece);
    if (pawn && !pawn->isPromoted() && pawn->canPromote(move)) {
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
      move.promoteType = selectedType;
    }
  }

  void ChessGame::processMove(const std::shared_ptr<Piece>& piece, const Move& move) {
    // This assumes that the move is already validated and generates the next state
    // and also checks if the next state would put the player in checkmate.
    LOG_PRINTF("Process move called\n");

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

    // Push performed action to stack
    m_ActionHistory.push_back({ dstSquare->occupyingPiece, move });

    // Check if current color is in checkmate
    bool opponentCheckmated = true;
    PieceColor opponentColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
    for (const std::shared_ptr<Piece> &piece : opponentColor == BLACK ? m_BlackPieces : m_WhitePieces) {
      std::vector<Move> possibleMoves = piece->getPossibleMoves(m_Board, m_ActionHistory);
      for (const Move& move : possibleMoves) {
        if (isValidMove(piece, move)) {
          opponentCheckmated = false;
          goto CHECKMATE_CHECK_EXIT;
        };
      }
    }

CHECKMATE_CHECK_EXIT:
    if (opponentCheckmated) {
      if (opponentColor == BLACK) {
        MessageBox(NULL, L"Black has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
        m_InProgress = false;
      }
      if (opponentColor == WHITE) {
        MessageBox(NULL, L"White has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
        m_InProgress = false;
      }
    }

    if (!m_Online) {
      // Swap player color for single player, otherwise turn is decided by network response
      m_PlayerColor = m_PlayerColor == BLACK ? WHITE : BLACK;
      m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
    }

    if (move.capturedPiece) {
      m_AudioHandler.playCaptureSound();
    }
    else {
      m_AudioHandler.playMoveSound();
    }

    m_RenderHandler.drawChessBoard(m_Board);
    m_RenderHandler.drawCapturedPieces();
  }

  void ChessGame::sendMove(const std::shared_ptr<Piece>& piece, const Move& move) {
    // Send move to server and validate
    m_DataBuffer.clear();
    m_DataBuffer.push_back(0x55);
    for (Square& square : m_Board) {
      if (square.occupyingPiece) {
        m_DataBuffer.push_back(getPieceKey(square.occupyingPiece));
      }
      else {
        m_DataBuffer.push_back(0);
      }
    }
    writeMove(piece, move);
    sendCommand();
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

    bool isValidMove = !isKingInCheck(boardCopy, piece->getColor());

    if (move.capturedPiece) {
      move.capturedPiece->setIsAlive(true);
    }

    return isValidMove;
  }

  bool ChessGame::isKingInCheck(std::array<Square, 64>& board, PieceColor color) {

    const std::set<std::shared_ptr<Piece>>& opponentsPieces = color == WHITE ? m_BlackPieces : m_WhitePieces;
    const std::shared_ptr<Piece>& myKing = color == WHITE ? m_WhiteKing : m_BlackKing;

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
    m_DataBuffer.push_back(getPieceKey(piece));
    m_DataBuffer.insert(m_DataBuffer.end(), moveBytes, moveBytes + sizeof(NetworkMove));
  }

  void ChessGame::sendCommand() {
    send(m_ClientSocket, reinterpret_cast<const char*>(m_DataBuffer.data()), static_cast<int>(m_DataBuffer.size()), 0);
    m_DataBuffer.clear();
  }
  
  Move ChessGame::decodeMove(unsigned char* data) {
    const NetworkMove* networkMove = reinterpret_cast<const NetworkMove*>(data);

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

  std::shared_ptr<Piece> ChessGame::getPiece(unsigned char pieceKey) {
    if (m_Pieces.find(pieceKey) != m_Pieces.end()) {
      return m_Pieces[pieceKey];
    }
    return nullptr;
  }

  bool ChessGame::validateBoard(unsigned char* data) {
    for (int i = 0; i < 64; i++) {
      unsigned char boardVal = m_Board[i].occupyingPiece ? getPieceKey(m_Board[i].occupyingPiece) : 0;
      if (boardVal != static_cast<unsigned char>(data[i])) {
        return false;
      }
    }
    return true;
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