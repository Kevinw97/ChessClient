# Defines CHESS_SERVER_BUILD and uses server-main.cpp as entry point

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -DCHESS_SERVER_BUILD -Wno-missing-field-initializers -Wno-unused-parameter
LDFLAGS = -lpthread

# Build directories
BUILD_DIR = build
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/chess_server

# Source directories
SRC_DIR = src
SERVER_DIR = $(SRC_DIR)/server

# Source files (exclude chess_main.cpp and SDL-related files for server)
SOURCES = $(SRC_DIR)/bishop.cpp \
          $(SRC_DIR)/chess_client.cpp \
          $(SRC_DIR)/chess_game.cpp \
          $(SRC_DIR)/king.cpp \
          $(SRC_DIR)/knight.cpp \
          $(SRC_DIR)/pawn.cpp \
          $(SRC_DIR)/piece.cpp \
          $(SRC_DIR)/queen.cpp \
          $(SRC_DIR)/rook.cpp \
          $(SERVER_DIR)/chess-server.cpp \
          $(SERVER_DIR)/server.cpp \
          $(SERVER_DIR)/server-main.cpp

# Object files (placed in build directory)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/server

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link the executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/chess_server

# Uninstall target (optional)
uninstall:
	rm -f /usr/local/bin/chess_server

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all     - Build the chess server (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug symbols"
	@echo "  install - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  help    - Show this help message"

.PHONY: all clean install uninstall debug help