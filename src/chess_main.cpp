#ifdef CHESS_CLIENT_BUILD
#include "chess_game.h"
#include <chrono>
#include <iostream>
#include <thread>

#define SDL_MAIN_HANDLED

int main() {
    chess_online::ChessGame game;
    game.run();
}
#endif
