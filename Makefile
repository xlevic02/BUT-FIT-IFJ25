CC = gcc
# CFLAGS=-Wall -Wextra -std=c99
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
BIN_DIR = bin


clean:
    rm -rf $(BUILD_DIR) $(BIN_DIR)
    
TARGET = $(BIN_DIR)/ifj
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%.test,$(TEST_SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
    $(CC) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
    $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(BIN_DIR):
    mkdir -p $@

# Build and run tests
test: all $(TEST_BINS)
    @for t in $(TEST_BINS); do \
        echo "Running $$t"; \
        ./$$t || exit 1; \
    done
    @echo "All tests passed."

$(BIN_DIR)/%.test: $(TEST_DIR)/%.c $(OBJS) | $(BIN_DIR)
    $(CC) $(CFLAGS) -I$(SRC_DIR) $< $(OBJS) -o $@

.PHONY: all clean test