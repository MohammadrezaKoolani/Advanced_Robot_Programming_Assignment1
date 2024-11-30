CC=gcc
CFLAGS=-Wall -Wextra -g -std=c11 -Iinclude  # Add -Iinclude to include the new header directory
LDFLAGS=-lncurses -lm

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

TARGET=bin/drone_simulator

# Create the build directory for object files
BUILD_DIR=build
OBJECTS=$(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target: compile the project
all: $(TARGET)

# Compile the target executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# Compile each source file into object files and place them in the build directory
$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)  # Ensure the build directory exists
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build directory and the binary
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Run the final executable
run: $(TARGET)
	./$(TARGET)
