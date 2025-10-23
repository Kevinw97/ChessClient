#pragma once
#include "piece.h"
#include "pawn.h"
#include "rook.h"
#include "knight.h"
#include "bishop.h"
#include "queen.h"
#include "king.h"
#include "sdl_handler.h"

namespace chess_client {
  class ChessGame {
  private:
    std::shared_ptr<Piece> m_WhitePieces[16];
    std::shared_ptr<Piece> m_BlackPieces[16];
    

  public:
    ChessGame();
  };

}  // namespace chess_client