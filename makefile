CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = bin/cpu

SRC = $(wildcard src/*.c)

OBJ = $(SRC:src/%.c=build/%.o)


.PHONY: all clean run

all: $(TARGET)


$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@


build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@


run: $(TARGET)
	./$(TARGET)


clean:
	rm -rf build bin