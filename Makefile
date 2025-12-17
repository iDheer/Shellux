# Makefile for Shellux - Custom Unix Shell
# Author: Inesh Dheer

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
LDFLAGS = 

# Directories
SRC_DIR = .
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SRCS = main.c commands.c utils.c globals.c prompt.c alias.c
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

# Target executable
TARGET = $(BIN_DIR)/shellux

# Default target
all: directories $(TARGET)

# Create directories if they don't exist
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "Clean complete"

# Full clean including directories
distclean: clean
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Distribution clean complete"

# Rebuild everything
rebuild: clean all

# Run the shell
run: all
	./$(TARGET)

# Install (copy to /usr/local/bin - requires sudo)
install: all
	sudo cp $(TARGET) /usr/local/bin/shellux
	@echo "Installed to /usr/local/bin/shellux"

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/shellux
	@echo "Uninstalled from /usr/local/bin/shellux"

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: clean all

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: clean all

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build the shell (default)"
	@echo "  clean     - Remove object files and executable"
	@echo "  distclean - Full clean including directories"
	@echo "  rebuild   - Clean and rebuild"
	@echo "  run       - Build and run the shell"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build with optimizations"
	@echo "  help      - Show this help message"

.PHONY: all clean distclean rebuild run install uninstall debug release help directories
