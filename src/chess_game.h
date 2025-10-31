#pragma once
#include "bishop.h"
#include "king.h"
#include "knight.h"
#include "pawn.h"
#include "piece.h"
#include "queen.h"
#include "rook.h"
#ifdef CHESS_CLIENT_BUILD
#include "chess_client.h"
#include "sdl_audio_handler.h"
#include "sdl_render_handler.h"
#endif
#include "chess.h"

namespace chess_online {
class ChessGame {
private:
    bool m_InProgress = true;
    bool m_Running = true;
    bool m_Checkmate = false;
    std::array<Square, NUM_SQUARES> m_Board;
#ifdef CHESS_CLIENT_BUILD
    RenderHandler m_RenderHandler;
    AudioHandler m_AudioHandler;
    ChessClient m_ChessClient;
    bool m_IsOnline = false;
    std::vector<Move> m_MovesForSelected;
#endif
#ifdef CHESS_SERVER_BUILD
    std::mutex m_Mutex;
#endif
    std::set<std::shared_ptr<Piece>> m_WhitePieces;
    std::set<std::shared_ptr<Piece>> m_BlackPieces;
    std::unordered_map<unsigned char, std::shared_ptr<Piece>> m_Pieces;
    std::shared_ptr<Piece> m_BlackKing = nullptr;
    std::shared_ptr<Piece> m_WhiteKing = nullptr;
    PieceColor m_PlayerColor = WHITE;
    PieceColor m_CurrentTurnColor = WHITE;
    Square *m_SelectedSquare = nullptr;
    std::vector<Action> m_ActionHistory;

#ifdef CHESS_SERVER_BUILD
    void generateInitialBoard(std::array<Square, 64> &board);
#endif
    void setupInitialPieces(std::array<Square, NUM_SQUARES> &board);

#ifdef CHESS_CLIENT_BUILD
    void gameSetup();
    void gameLoop();
    void handleMouseClick(SDL_Event *event);
    void clientMoveHandler(
        unsigned char pieceKey,
        NetworkMove networkMove,
        std::array<unsigned char, NUM_SQUARES> serializedBoard,
        PieceColor turnColor);
    void selectSource(int x, int y);
    void selectDestination(int x, int y);
    void choosePawnPromotion(const std::shared_ptr<Piece> &piece, Move &move);
    void unselectAllSquares();
    void undoMove();
    void resetGame();
#endif
public:
    ChessGame();
#ifdef CHESS_CLIENT_BUILD
    void run();
#endif
#ifdef CHESS_SERVER_BUILD
    std::mutex &getMutex() { return m_Mutex; };
#endif
    bool isValidMove(const std::shared_ptr<Piece> &piece, const Move &move);
    bool isKingInCheck(std::array<Square, NUM_SQUARES> &board, PieceColor Color);
    bool validateBoard(std::array<unsigned char, NUM_SQUARES> board);
    bool isCurrentPlayersTurn();
    void processMove(const std::shared_ptr<Piece> &piece, const Move &move);
    bool isCheckmate();
    std::shared_ptr<Piece> getPiece(unsigned char pieceKey);
    PieceColor getTurn();
    std::array<unsigned char, NUM_SQUARES> serializeBoard();
    Move decodeMove(NetworkMove data);
};

} // namespace chess_online