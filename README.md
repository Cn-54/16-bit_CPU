# 16-bit CPU

A simple 16-bit CPU emulator written in C.

The project includes a custom instruction set, assembler, byte-addressable memory, stack, vector table, and a small set of assembly subroutines.

## Features

* 16 × 16-bit registers
* 16-bit addresses
* 64 KiB byte-addressable memory
* 32-bit fixed-size instructions
* Stack with `PUSH`, `POP`, `CALL` and `RET`
* Custom assembler with labels, `.ORG` and `DB`
* Vector table and system calls
* Assembly subroutines for `PRINT`, `STRLEN`, `ATOI` and `ITOA`

See the [ISA documentation](Docs/ISA.md) for the full instruction set and architecture.

## Usage

Build the CPU:

```bash
make cpu
```

Build the assembler:

```bash
make assembler
```

Assemble a program:

```bash
./bin/assembler Code/programs/program.A Code/programs/program.bin
```

Run a program:

```bash
./bin/cpu Code/programs/program.bin
```

## Structure

```text
.
├── Code
│   ├── programs
│   │   ├── program.A
│   │   └── program.bin
│   ├── subroutines
│   │   ├── atoi
│   │   │   ├── atoi.A
│   │   │   └── atoi.bin
│   │   ├── itoa
│   │   │   ├── itoa.A
│   │   │   └── itoa.bin
│   │   ├── print
│   │   │   ├── print.bin
│   │   │   └── print.hex
│   │   ├── strlen
│   │   │   ├── strlen.A
│   │   │   └── strlen.bin
│   │   └── subroutine
│   └── vector_table
│       ├── vec_table.bin
│       └── vec_table.hex
├── compile_flags.txt
├── docs
│   └── ISA.md
├── include
│   ├── cpu.h
│   ├── data.h
│   ├── instruction.h
│   ├── loader.h
│   ├── memory.h
│   └── opcode.h
├── makefile
├── src
│   ├── Loader.c
│   ├── Memory.c
│   ├── assemble.c
│   ├── cpu.c
│   └── main.c
└── utils
    └── converter.py
```
