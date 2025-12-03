# Název výsledného programu
TARGET = wren_compiler

# Zdrojové soubory
SRCS = ast.c error.c generator.c main.c scanner.c sem_analysis.c symtable.c

# Z objektů se vytvoří binárka
OBJS = $(SRCS:.c=.o)

# Kompilátor a flagy
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Implicitní cíl
all: $(TARGET)

# Linkování
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@rm -f $(OBJS)

# Obecné pravidlo pro .o z .c
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

# main.c nemá vlastní hlavičku
main.o: main.c
	$(CC) $(CFLAGS) -c $<

# Úklid
clean:
	rm -f $(OBJS) $(TARGET)
