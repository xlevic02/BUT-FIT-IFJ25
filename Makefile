# === Makefile ===

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -I./src

# Directories
SRC_DIR := src
BUILD_DIR := build
INPUT_DIR := example_wren_files
OUTPUT_DIR := output
TESTS_DIR := tests

# Files
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET := $(BUILD_DIR)/wren_compiler
INTERPRET := $(TESTS_DIR)/tools/ic25int-linux-x86_64

# Default target
all: $(TARGET)

# Build binary
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Compile .c files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run with input file (usage: make run FILE=ex1-faktorial-iterativne.wren)
run: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<filename.wren>"; \
	else \
		rm -f $(OUTPUT_DIR)/$(FILE:.wren=.out); \
		$(TARGET) < $(INPUT_DIR)/$(FILE) > $(OUTPUT_DIR)/$(FILE:.wren=.out); \
	fi

Compile:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<filename.wren>"; \
	else \
		$(INTERPRET) $(OUTPUT_DIR)/$(FILE); \
	fi 

test:
	$(TESTS_DIR)/run_all_tests.sh
	
fix-perms:
	chmod +rwx $(TESTS_DIR)/*.sh
	chmod +rwx $(TESTS_DIR)/tools/ic25int-linux-x86_64
	chmod +rwx $(BUILD_DIR)/wren_compiler

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run
