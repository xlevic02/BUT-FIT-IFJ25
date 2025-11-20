CC = gcc
# CFLAGS=-Wall -Wextra -std=c99
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INPUT_DIR = example_wren_files
OUTPUT_DIR = output
TARGET = $(BIN_DIR)/scanner

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# Run with input file (usage: make run FILE=ex1-faktorial-iterativne.wren)
run: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<filename.wren>"; \
	else \
		rm -f $(OUTPUT_DIR)/$(FILE:.wren=.out); \
		$(TARGET) < $(INPUT_DIR)/$(FILE) > $(OUTPUT_DIR)/$(FILE:.wren=.out); \
	fi

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean
