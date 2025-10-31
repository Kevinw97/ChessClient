# Cpp Chess

Simple Chess game written in C++ with online capabilities. Uses SDL3 for rendering/audio/event handling.
Implements special moves such as castling, en passante, and pawn promotion.
Can undo moves using left arrow, and reset the game by pressing F4.

Client is built in Visual Studio with the solution. Include paths, libraries for SDL3 need to be set up.
Server is built using the Makefile

Multiple clients can connect to a running server instance and they get matched based on first in first out. 
When running it online, the client sends the moves along with the board state to the server to validate, and the server will process the move, send the move back, and attach the resulting board, where it is further validated on client.

## How to build and run:
Can be built in Visual Studio using the solution. Install SDL3 from https://github.com/libsdl-org/SDL/releases with the VC devel package and follow the install.md there. Requires the SDL3 libraries/headers/dll.
