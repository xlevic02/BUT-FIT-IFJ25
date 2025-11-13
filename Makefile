# === Makefile ===

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -I./src

# Directories
SRC_DIR := src
BUILD_DIR := build
INPUT_DIR := inputs

# Files
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET := $(BUILD_DIR)/wren_compiler

# Default target
all: $(TARGET)

# Build binary
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Compile .c files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run with input file (usage: make run FILE=ex1-faktorial-iterativne.wren)
run: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<filename.wren>"; \
	else \
		$(TARGET) < $(INPUT_DIR)/$(FILE); \
	fi

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run
