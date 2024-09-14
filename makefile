# Compiler
CC = gcc

# Source files
SRCS = main.c prompt.c commands.c utils.c init.c

# Output executable
TARGET = a.out

# Default rule
all: $(TARGET)

# Rule for building the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean up build files
clean:
	rm -f $(TARGET)
