#include "chess_game.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <chrono>
#include <iostream>
#include <thread>

#define SDL_MAIN_HANDLED

int main() {
    chess_client::ChessGame game;
    game.run();
}
