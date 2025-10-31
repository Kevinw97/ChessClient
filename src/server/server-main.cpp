#ifdef CHESS_SERVER_BUILD
#include "chess-server.h"
#include "helpers.h"
#include "server.h"
#include <iostream>
#include <sstream>
#include <thread>

int main() {
    chess_online::ChessServer ChessServer;
    ChessServer.run();
}
#endif