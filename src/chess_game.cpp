#include "chess_game.h"
#include "sdl_audio_handler.h"
#include "sdl_render_handler.h"
#include <iostream>
#include <thread>

namespace chess_online {

ChessGame::ChessGame()
    : m_Board{}
#ifdef CHESS_CLIENT_BUILD
      ,
      m_RenderHandler("Chess Game")
#endif
{
#ifdef CHESS_CLIENT_BUILD
    m_RenderHandler.generateInitialBoard(m_Board);
#endif
#ifdef CHESS_SERVER_BUILD
    generateInitialBoard(m_Board);
#endif
    setupInitialPieces(m_Board);
}

#ifdef CHESS_SERVER_BUILD
void ChessGame::generateInitialBoard(std::array<Square, 64> &board) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int i = posToIndex({x, y});
            board[i].pos = {x, y};
        }
    }
}
#endif

void ChessGame::setupInitialPieces(std::array<Square, NUM_SQUARES> &board) {
    for (int i = 0; i < 8; i++) {
        m_WhitePieces.insert(std::make_shared<Pawn>(&board[posToIndex({i, 6})], WHITE));
        m_BlackPieces.insert(std::make_shared<Pawn>(&board[posToIndex({i, 1})], BLACK));
    }
    m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({0, 7})], WHITE));
    m_WhitePieces.insert(std::make_shared<Rook>(&board[posToIndex({7, 7})], WHITE));
    m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({1, 7})], WHITE));
    m_WhitePieces.insert(std::make_shared<Knight>(&board[posToIndex({6, 7})], WHITE));
    m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({2, 7})], WHITE));
    m_WhitePieces.insert(std::make_shared<Bishop>(&board[posToIndex({5, 7})], WHITE));
    m_WhitePieces.insert(std::make_shared<Queen>(&board[posToIndex({3, 7})], WHITE));
    m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({0, 0})], BLACK));
    m_BlackPieces.insert(std::make_shared<Rook>(&board[posToIndex({7, 0})], BLACK));
    m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({1, 0})], BLACK));
    m_BlackPieces.insert(std::make_shared<Knight>(&board[posToIndex({6, 0})], BLACK));
    m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({2, 0})], BLACK));
    m_BlackPieces.insert(std::make_shared<Bishop>(&board[posToIndex({5, 0})], BLACK));
    m_BlackPieces.insert(std::make_shared<Queen>(&board[posToIndex({3, 0})], BLACK));

    m_WhiteKing = std::make_shared<King>(&board[posToIndex({4, 7})], WHITE);
    m_BlackKing = std::make_shared<King>(&board[posToIndex({4, 0})], BLACK);

    m_WhitePieces.insert(m_WhiteKing);
    m_BlackPieces.insert(m_BlackKing);

    for (const auto &piece : m_WhitePieces) {
        int index = posToIndex(piece->getSquare()->pos);
        m_Board[index].occupyingPiece = piece;
        m_Pieces.emplace(piece->getPieceKey(), piece);
    }

    for (const auto &piece : m_BlackPieces) {
        int index = posToIndex(piece->getSquare()->pos);
        m_Board[index].occupyingPiece = piece;
        m_Pieces.emplace(piece->getPieceKey(), piece);
    }
}
#ifdef CHESS_CLIENT_BUILD
void ChessGame::gameSetup() {
    std::cout << "Choose singleplayer[s] or online[m]: ";
    std::string userInput;
    while (std::getline(std::cin, userInput)) {
        std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower);
        if (userInput == "s") {
            m_IsOnline = false;
            return;
        } else if (userInput == "m") {
            m_IsOnline = true;
            return;
        } else {
            std::cout << "Invalid input. Choose singleplayer[s] or online[m]: ";
        }
    }
}

void ChessGame::gameLoop() {
    SDL_Event event;
    while (m_InProgress && SDL_WaitEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            m_Running = false;
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
            if (!m_IsOnline && event.key.key == SDLK_F4) {
                m_InProgress = false;
            }
            if (!m_IsOnline && event.key.key == SDLK_LEFT) {
                undoMove();
            }
            break;
        }
        }
    }
}

void ChessGame::run() {
    gameSetup();
    if (m_IsOnline) {
        m_ChessClient.initClient([this](
                                     unsigned char pieceKey,
                                     NetworkMove networkMove,
                                     std::array<unsigned char, NUM_SQUARES> serializedBoard,
                                     PieceColor color) {
            clientMoveHandler(pieceKey, networkMove, serializedBoard, color);
        });

        m_PlayerColor = m_ChessClient.getAssignedColor();
        if (m_PlayerColor == BLACK) {
            m_RenderHandler.rotateBoard(m_Board);
        }
    }

    m_RenderHandler.init();
    m_AudioHandler.init();

    m_RenderHandler.drawChessBoard(m_Board);

    while (m_Running) {
        if (m_InProgress) {
            gameLoop();
        } else {
            if (!m_IsOnline) {
                resetGame();
            }
        }
    }
}

void ChessGame::handleMouseClick(SDL_Event *event) {
    // Only handle click events for current player's turn
    Position pos = m_RenderHandler.mouseToPosition(event);
    if (isValidPosition(pos)) {
        if (!m_SelectedSquare) {
            selectSource(pos.x, pos.y);
        } else {
            selectDestination(pos.x, pos.y);
        }
    }
}

void ChessGame::selectSource(int x, int y) {
    Square *clickedSquare = getSquareAtPosition(m_Board, {x, y});
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
    for (const Move &move : possibleMoves) {
        if (!isValidMove(m_SelectedSquare->occupyingPiece, move)) {
            continue;
        };
        Square *targetSquare = getSquareAtPosition(m_Board, move.dst);
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
                                          [&](Move &move) { return move.dst.x == x && move.dst.y == y; });

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

    if (m_IsOnline) {
        m_ChessClient.sendMove(serializeBoard(), piece, move);
    } else {
        processMove(piece, move);
    }

    return;
}

void ChessGame::choosePawnPromotion(const std::shared_ptr<Piece> &piece, Move &move) {
    auto pawn = std::dynamic_pointer_cast<Pawn>(piece);
    if (pawn && !pawn->isPromoted() && pawn->canPromote(move)) {
        MessageBox(NULL, L"You can promote this pawn, choose a piece in the console.", L"Chess", MB_OK | MB_ICONINFORMATION);
        std::cout << "Pawn promotion. Choose a piece (rook, bishop, knight, queen): ";
        std::string pieceInput;
        PieceType selectedType = QUEEN; // Default to queen
        while (std::getline(std::cin, pieceInput)) {
            std::transform(pieceInput.begin(), pieceInput.end(), pieceInput.begin(), ::tolower);
            if (pieceInput == "rook") {
                selectedType = ROOK;
                break;
            } else if (pieceInput == "bishop") {
                selectedType = BISHOP;
                break;
            } else if (pieceInput == "knight") {
                selectedType = KNIGHT;
                break;
            } else if (pieceInput == "queen") {
                selectedType = QUEEN;
                break;
            } else {
                std::cout << "Invalid input. Choose rook, bishop, knight, or queen: ";
            }
        }
        move.promoteType = selectedType;
    }
}

void ChessGame::clientMoveHandler(
    unsigned char pieceKey,
    NetworkMove networkMove,
    std::array<unsigned char, NUM_SQUARES> serializedBoard,
    PieceColor turnColor) {
    std::shared_ptr<Piece> piece = getPiece(pieceKey);
    Move move = decodeMove(networkMove);
    if (!isValidMove(piece, move)) {
        m_Running = false;
        return;
    }

    processMove(piece, move);
    if (!validateBoard(serializedBoard)) {
        m_Running = false;
        return;
    }

    LOG_PRINTF("Turn is: %x\n", turnColor);
    m_CurrentTurnColor = turnColor;
}

void ChessGame::unselectAllSquares() {
    for (Square &square : m_Board) {
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

    Square *srcSquare = piece->getSquare();
    Square *dstSquare = getSquareAtPosition(m_Board, move.dst);

    piece->performMove(m_Board, move);
    dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);

    // If no more moves are found for this piece, we are undoing it's first move
    if (std::find_if(m_ActionHistory.begin(), m_ActionHistory.end(),
                     [&](const Action &action) { return action.piece == piece; }) == m_ActionHistory.end()) {
        piece->setMoved(false);
    }

    if (move.capturedPiece) {
        // Revive piece that was captured, it should still reference
        // the square that it was captured from
        Square *capturedSquare = move.capturedPiece->getSquare();
        capturedSquare->occupyingPiece = move.capturedPiece;

        move.capturedPiece->setIsAlive(true);
        m_RenderHandler.undoCapture(move.capturedPiece->getColor());
    }

    // Undo castling
    if (move.castlingRook && isValidPosition(move.castlingRookDst)) {
        Square *rookSrcSquare = move.castlingRook->getSquare();
        Square *rookDstSquare = getSquareAtPosition(m_Board, move.castlingRookDst);

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

    m_RenderHandler.drawChessBoard(m_Board);
    m_RenderHandler.drawCapturedPieces();
}

void ChessGame::resetGame() {
    for (Square &square : m_Board) {
        square.occupyingPiece = nullptr;
    }
    for (auto &piece : m_WhitePieces) {
        piece->resetPiece(m_Board);
        Square *initialSquare = getSquareAtPosition(m_Board, piece->getInitialPosition());
        initialSquare->occupyingPiece = piece;
    }
    for (auto &piece : m_BlackPieces) {
        piece->resetPiece(m_Board);
        Square *initialSquare = getSquareAtPosition(m_Board, piece->getInitialPosition());
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

#endif

void ChessGame::processMove(const std::shared_ptr<Piece> &piece, const Move &move) {
    Square *srcSquare = piece->getSquare();
    Square *dstSquare = getSquareAtPosition(m_Board, move.dst);

    if (move.capturedPiece) {
        // Captured piece is not always the destination square in moves like en passante
        Square *capturedSquare = move.capturedPiece->getSquare();
        capturedSquare->occupyingPiece = nullptr;

        move.capturedPiece->setIsAlive(false);
#ifdef CHESS_CLIENT_BUILD
        m_RenderHandler.capturePiece(move.capturedPiece);
#endif
    }

    piece->performMove(m_Board, move);
    dstSquare->occupyingPiece = std::move(srcSquare->occupyingPiece);

    // Castling
    if (move.castlingRook && isValidPosition(move.castlingRookDst)) {
        Square *rookSrcSquare = move.castlingRook->getSquare();
        Square *rookDstSquare = getSquareAtPosition(m_Board, move.castlingRookDst);

        move.castlingRook->performMove(m_Board, move);
        rookDstSquare->occupyingPiece = std::move(rookSrcSquare->occupyingPiece);
    }

    // Push performed action to stack
    m_ActionHistory.push_back({dstSquare->occupyingPiece, move});

    // Check if opponent color is in checkmate
    bool opponentCheckmated = true;
    PieceColor opponentColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
    for (const std::shared_ptr<Piece> &piece : opponentColor == BLACK ? m_BlackPieces : m_WhitePieces) {
        std::vector<Move> possibleMoves = piece->getPossibleMoves(m_Board, m_ActionHistory);
        for (const Move &move : possibleMoves) {
            if (isValidMove(piece, move)) {
                opponentCheckmated = false;
                goto CHECKMATE_CHECK_EXIT;
            };
        }
    }

CHECKMATE_CHECK_EXIT:
    if (opponentCheckmated) {
#ifdef CHESS_CLIENT_BUILD
        if (opponentColor == BLACK) {
            MessageBox(NULL, L"Black has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
            m_InProgress = false;
        }
        if (opponentColor == WHITE) {
            MessageBox(NULL, L"White has been checkmated.", L"Chess", MB_OK | MB_ICONINFORMATION);
            m_InProgress = false;
        }
#else
        if (m_CurrentTurnColor == BLACK) {
            LOG_COUT("Black has been checkmated.");
            m_InProgress = false;
        }
        if (m_CurrentTurnColor == WHITE) {
            LOG_COUT("White has been checkmated.");
            m_InProgress = false;
        }
#endif
        m_Checkmate = true;
    }

#ifdef CHESS_CLIENT_BUILD
    if (!m_IsOnline) {
        // Swap player color for single player, otherwise turn is decided by network response
        m_PlayerColor = m_PlayerColor == BLACK ? WHITE : BLACK;
        m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
    }

    if (move.capturedPiece) {
        m_AudioHandler.playCaptureSound();
    } else {
        m_AudioHandler.playMoveSound();
    }

    m_RenderHandler.drawChessBoard(m_Board);
    m_RenderHandler.drawCapturedPieces();
#endif
#ifdef CHESS_SERVER_BUILD
    // Switch turn
    m_CurrentTurnColor = m_CurrentTurnColor == BLACK ? WHITE : BLACK;
#endif
}

bool ChessGame::isValidMove(const std::shared_ptr<Piece> &piece, const Move &move) {
    // Soft check for whether this piece can actually move to this position
    auto allPossibleMoves = piece->getPossibleMoves(m_Board, m_ActionHistory);
    if (std::find_if(allPossibleMoves.begin(), allPossibleMoves.end(),
                     [&](const Move &possibleMove) {
                         return possibleMove.src.x == move.src.x &&
                                possibleMove.src.y == move.src.y &&
                                possibleMove.dst.x == move.dst.x &&
                                possibleMove.dst.y == move.dst.y;
                     }) == allPossibleMoves.end()) {
        return false;
    }

    // Create copy of current board and check the board state if the selected piece was at pos
    std::array<Square, NUM_SQUARES> boardCopy = m_Board;

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

bool ChessGame::isKingInCheck(std::array<Square, NUM_SQUARES> &board, PieceColor color) {

    const std::set<std::shared_ptr<Piece>> &opponentsPieces = color == WHITE ? m_BlackPieces : m_WhitePieces;
    const std::shared_ptr<Piece> &myKing = color == WHITE ? m_WhiteKing : m_BlackKing;

    for (const auto &opponentPiece : opponentsPieces) {
        std::vector<Move> possibleMoves = opponentPiece->getPossibleMoves(board, m_ActionHistory);
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
    return m_CurrentTurnColor == m_PlayerColor;
}

bool ChessGame::isCheckmate() {
    return m_Checkmate;
}

Move ChessGame::decodeMove(NetworkMove networkMove) {
    // Decode into a Move struct
    Move move;
    move.src = networkMove.src;
    move.dst = networkMove.dst;
    move.castlingRookSrc = networkMove.castlingRookSrc;
    move.castlingRookDst = networkMove.castlingRookDst;
    move.promoteType = networkMove.promoteType;
    move.firstMove = networkMove.firstMove;

    // Decode piece keys back to shared_ptr<Piece>
    if (networkMove.capturedPiece) {
        unsigned char key = networkMove.capturedPiece;
        move.capturedPiece = m_Pieces[key];
    }

    if (networkMove.castlingRook) {
        unsigned char key = networkMove.castlingRook;
        move.castlingRook = m_Pieces[key];
    }

    return move;
}

std::shared_ptr<Piece> ChessGame::getPiece(unsigned char pieceKey) {
    if (m_Pieces.find(pieceKey) != m_Pieces.end()) {
        return m_Pieces[pieceKey];
    }
    return nullptr;
}

PieceColor ChessGame::getTurn() {
    return m_CurrentTurnColor;
}

bool ChessGame::validateBoard(std::array<unsigned char, NUM_SQUARES> serializedBoard) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        unsigned char boardVal = m_Board[i].occupyingPiece ? m_Board[i].occupyingPiece->getPieceKey() : 0;
        if (boardVal != static_cast<unsigned char>(serializedBoard[i])) {
            return false;
        }
    }
    return true;
}

std::array<unsigned char, NUM_SQUARES> ChessGame::serializeBoard() {
    std::array<unsigned char, NUM_SQUARES> serializedBoard;
    for (int i = 0; i < NUM_SQUARES; i++) {
        serializedBoard[i] = m_Board[i].occupyingPiece ? m_Board[i].occupyingPiece->getPieceKey() : 0;
    }
    return serializedBoard;
}
} // namespace chess_online