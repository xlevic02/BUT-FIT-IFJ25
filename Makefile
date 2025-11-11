CC = gcc
# CFLAGS=-Wall -Wextra -std=c99
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror

SRC_DIR = src
TEST_SRC_DIR = tests/src
BUILD_DIR = build
BIN_DIR = bin

TARGET = $(BIN_DIR)/scanner
#TODO edit TEST_TARGET for multiple tests
TEST_TARGET = $(BIN_DIR)/lexer_tests

SRC ?= tests/src/test_make_src
EXPECT ?= tests/src/test_make

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TEST_SRCS = $(wildcard $(TEST_SRC_DIR)/*.c)
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.c,$(BUILD_DIR)/test_%.o,$(TEST_SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%.o: $(TEST_SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

test: $(TEST_TARGET)
	@echo "=== Running tests ==="
	@$(TEST_TARGET) $(SRC) $(EXPECT)

# Objects without main.c (for tests)
OBJS_NO_MAIN = $(filter-out $(BUILD_DIR)/main.o,$(OBJS))

# Build test executable
$(TEST_TARGET): $(OBJS_NO_MAIN) $(TEST_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean test
