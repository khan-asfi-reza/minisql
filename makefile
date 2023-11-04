# Makefile

CC = gcc
CFLAGS = -std=c23 -Wall
TARGET = build/minisql
SRCS = src/main.c src/lexer.c src/const.c src/database.c src/filesystem.c src/io.c src/util.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
