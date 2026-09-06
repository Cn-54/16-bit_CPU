# 16-bit CPU ISA

## Architecture

| Feature            | Value                   |
| ------------------ | ----------------------- |
| Word size          | 16-bit                  |
| Address size       | 16-bit                  |
| Memory             | 64 KiB                  |
| Memory addressing  | Byte-addressable        |
| Registers          | 16 × 16-bit             |
| Instruction size   | 32-bit                  |
| Instruction format | `OPCODE DEST SRC1 SRC2` |
| Endianness         | Big-endian words        |
| Stack              | Downward-growing        |
| Stack pointer      | `SP`                    |
| Program counter    | `PC`                    |

Each instruction is 4 bytes:

```text
┌────────┬────────┬────────┬────────┐
│ OPCODE │  DEST  │  SRC1  │  SRC2  │
│  8-bit │  8-bit │  8-bit │  8-bit │
└────────┴────────┴────────┴────────┘
```

Instructions are fetched from memory at `PC` and `PC` is incremented by 4 before the instruction is executed.

## Registers

The CPU has 16 general-purpose registers:

```text
R0 - R15
```

All registers are 16-bit.

Special registers:

```text
PC      Program Counter
SP      Stack Pointer
FLAGS   Status flags
```

### Initial state

```text
PC = 0x1000
SP = 0xFFFF
FLAGS = 0
```

## Memory

Memory is:

```text
0x0000 - 0xFFFF
```

for a total of 64 KiB.

Memory is byte-addressable.

Words are stored big-endian:

```text
Address    Value
0x1000     high byte
0x1001     low byte
```

### Memory layout

```text
0x0000 ─────────────── Vector table

0x0100 ─────────────── Subroutines

0x1000 ─────────────── Program

0xFF00 ─────────────── Stack

0xFFFF
```

## Instruction Set

### System

| Opcode | Instruction | Description                 |
| ------ | ----------- | --------------------------- |
| `0x00` | `NOP`       | No operation                |
| `0x01` | `HALT`      | Halt the CPU                |
| `0x02` | `SYSCALL`   | Call a vector-table handler |

### Data Operations

| Opcode | Instruction | Description                                 |
| ------ | ----------- | ------------------------------------------- |
| `0x10` | `MOV`       | Copy a register                             |
| `0x11` | `LOAD`      | Load a 16-bit word from memory              |
| `0x12` | `STORE`     | Store a 16-bit word to memory               |
| `0x13` | `PUSH`      | Push a register onto the stack              |
| `0x14` | `POP`       | Pop a word from the stack                   |
| `0x15` | `MOVI`      | Move a 16-bit immediate into a register     |
| `0x16` | `LOADB`     | Load a byte from memory                     |
| `0x17` | `STOREB`    | Store a byte to memory                      |
| `0x18` | `INC`       | Increment a register                        |
| `0x19` | `DEC`       | Decrement a register                        |
| `0x1A` | `LOADIND`   | Load a word using a register as an address  |
| `0x1B` | `STOREIND`  | Store a word using a register as an address |
| `0x1C` | `LOADBIND`  | Load a byte using a register as an address  |
| `0x1D` | `STOREBIND` | Store a byte using a register as an address |

### Arithmetic

| Opcode | Instruction | Description            |
| ------ | ----------- | ---------------------- |
| `0x20` | `ADD`       | Add two registers      |
| `0x21` | `SUB`       | Subtract two registers |
| `0x22` | `MUL`       | Multiply two registers |
| `0x23` | `DIV`       | Divide two registers   |

### Logic

| Opcode | Instruction | Description |
| ------ | ----------- | ----------- |
| `0x30` | `AND`       | Bitwise AND |
| `0x31` | `OR`        | Bitwise OR  |
| `0x32` | `XOR`       | Bitwise XOR |
| `0x33` | `NOT`       | Bitwise NOT |

### Comparison

| Opcode | Instruction | Description           |
| ------ | ----------- | --------------------- |
| `0x40` | `CMP`       | Compare two registers |

`CMP` compares `SRC1` and `SRC2` and sets one of the comparison flags:

```text
SRC1 == SRC2 → Z
SRC1 >  SRC2 → G
SRC1 <  SRC2 → L
```

### Control Flow

| Opcode | Instruction | Description                 |
| ------ | ----------- | --------------------------- |
| `0x50` | `JMP`       | Unconditional relative jump |
| `0x51` | `JE`        | Jump if equal               |
| `0x52` | `JNE`       | Jump if not equal           |
| `0x53` | `JG`        | Jump if greater             |
| `0x54` | `JL`        | Jump if less                |
| `0x55` | `CALL`      | Call a relative address     |
| `0x56` | `RET`       | Return from a subroutine    |

Jumps and calls use a signed 16-bit PC-relative offset.

The offset is applied **after** the normal 4-byte instruction increment:

```text
PC = PC + 4
PC = PC + offset
```

For example:

```asm
JMP -8
```

jumps backwards 8 bytes from the address of the next instruction.

`CALL` pushes the current `PC` onto the stack before applying the relative offset.

`RET` restores the address from the stack.

### I/O

| Opcode | Instruction | Description        |
| ------ | ----------- | ------------------ |
| `0x60` | `PUTC`      | Output a character |
| `0x61` | `INP`      | Reads a character from the terminal |

`PUTC` outputs the low 8 bits of the specified register.

Example:

```asm
MOVI R1, 65
PUTC R1
```

Outputs:

```text
A
```

## Instruction Syntax

### Register operations

```asm
MOV R0, R1
ADD R0, R1, R2
SUB R0, R1, R2
MUL R0, R1, R2
DIV R0, R1, R2
AND R0, R1, R2
OR R0, R1, R2
XOR R0, R1, R2
NOT R0, R1
```

### Immediate values

```asm
MOVI R0, 1234
```

The immediate value is a full 16-bit value.

### Memory

Direct:

```asm
LOAD R0, 0x2000
STORE 0x2000, R0

LOADB R0, 0x2000
STOREB 0x2000, R0
```

Indirect:

```asm
LOADIND R0, R1
STOREIND R0, R1

LOADBIND R0, R1
STOREBIND R0, R1
```

For indirect operations, the address is stored in a register.

### Stack

```asm
PUSH R0
POP R0
```

The stack grows downwards from `SP`.

### Comparison and branches

```asm
CMP R0, R1

JE LABEL
JNE LABEL
JG LABEL
JL LABEL
JMP LABEL
```

Labels are resolved by the assembler into signed PC-relative offsets.

## Flags

`FLAGS` is a 16-bit register.

| Bit    | Name     | Description  |
| ------ | -------- | ------------ |
| `0`    | `Z`      | Zero / equal |
| `1`    | `G`      | Greater than |
| `2`    | `L`      | Less than    |
| `3`    | `N`      | Negative     |
| `4`    | `V`      | Overflow     |
| `5`    | `U`      | Underflow    |
| `6`    | `H`      | Halted       |
| `7`    | `D`      | Debug        |
| `8-15` | Reserved | Unused       |

Currently, `CMP` uses `Z`, `G` and `L`.

`H` is set when `HALT` is executed.

`D` enables CPU debug output.

## Vector Table

The vector table begins at:

```text
0x0000
```

Each vector is a 16-bit address.

```text
vector address = vector number × 2
```

For example:

```asm
SYSCALL 0
```

loads the handler address from:

```text
memory[0x0000]
```

`SYSCALL 1` loads from:

```text
memory[0x0002]
```

Current vectors:

| Vector | Handler  | Address  |
| ------ | -------- | -------- |
| `0`    | `PRINT`  | `0x0100` |
| `1`    | `STRLEN` | `0x0200` |
| `2`    | `ATOI`   | `0x0300` |
| `3`    | `ITOA`   | `0x0400` |

`SYSCALL` saves the return address on the stack before jumping to the handler.

## Assembler

The assembler supports:

* Registers `R0-R15`
* Decimal and hexadecimal values
* Labels
* PC-relative jumps
* PC-relative calls
* `.ORG`
* `DB`
* Quoted strings
* Escape sequences
* Comments using `;`

### `.ORG`

`.ORG` sets the address used by the assembler without adding padding to the output file.

```asm
.ORG 0x1000

MOVI R0, MESSAGE

MESSAGE:
    DB "Hello"
```

### `DB`

`DB` emits raw bytes.

Strings are automatically null-terminated:

```asm
DB "Hello"
```

Escape sequences include:

```text
\n
\t
\r
\\
\"
\0
```

## Runtime Subroutines

### PRINT

```asm
SYSCALL 0
```

Prints a null-terminated string.

Input:

```text
R0 = string address
R2 = 0
```

### STRLEN

```asm
SYSCALL 1
```

Returns the length of a null-terminated string.

Input:

```text
R0 = string address
```

Output:

```text
R1 = string length
```

### ATOI

```asm
SYSCALL 2
```

Converts an ASCII decimal string into an integer.

Input:

```text
R0 = string address
```

Output:

```text
R0 = integer
```

### ITOA

```asm
SYSCALL 3
```

Converts an unsigned integer into an ASCII decimal string.

Input:

```text
R0 = integer
R1 = destination buffer
```

Output:

```text
R1 = string address
```
