CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

CPU = bin/cpu
ASSEMBLER = bin/assembler

CPU_SRC = $(filter-out src/assemble.c,$(wildcard src/*.c))
CPU_OBJ = $(CPU_SRC:src/%.c=build/%.o)

.PHONY: cpu assembler clean

cpu: $(CPU)

assembler: $(ASSEMBLER)

$(CPU): $(CPU_OBJ)
	@mkdir -p bin
	$(CC) $(CPU_OBJ) -o $@

$(ASSEMBLER): src/assemble.c
	@mkdir -p bin
	$(CC) $(CFLAGS) src/assemble.c -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin