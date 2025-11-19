CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRC = main.c \
      ast.c \
      error.c \
      generator.c \
      scanner.c \
      symtable.c

OBJ = $(SRC:.c=.o)

TARGET = wren_compiler

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

# main.c nemá header, proto speciální pravidlo
main.o: main.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
