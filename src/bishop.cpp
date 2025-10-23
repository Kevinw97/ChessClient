#include "bishop.h"
namespace chess_client
{
  Bishop::Bishop(int x, int y, bool isBlack)
    : Piece(x, y, isBlack)
  {
    loadSurface(isBlack ? "res/b_bishop.png" : "res/w_bishop.png");
  };
  std::vector<Position> Bishop::getPossibleMoves()
  {
    std::vector<Position> moves;
    // Bishop movement logic to be implemented
    return moves;
  };
}  // namespace chess_client