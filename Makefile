
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
TARGET = bin/main

SRC = src/main.c src/command_func.c \
      src/custom_command.c src/signal_func.c

OBJ = $(SRC:src/%.c=build/src/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

build/src/%.o: src/%.c
	mkdir -p build/src
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin
