CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRC=main.c scanner.c
BIN=scanner

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN)

.PHONY: all run clean
