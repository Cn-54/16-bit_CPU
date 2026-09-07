# 16-bit CPU ISA

A 16-bit CPU architecture implemented in C with a custom assembler, byte-addressable memory, a fixed-width instruction set, software vectors, hardware-style terminal interrupts, and a downward-growing stack.

---

## Architecture

| Feature            | Value                   |
| ------------------ | ----------------------- |
| Word size          | 16-bit                  |
| Address size       | 16-bit                  |
| Memory             | 64 KiB                  |
| Memory addressing  | Byte-addressable        |
| Registers          | 16 × 16-bit             |
| Instruction size   | 32-bit / 4 bytes        |
| Instruction format | `OPCODE DEST SRC1 SRC2` |
| Endianness         | Big-endian words        |
| Stack              | Downward-growing        |
| Program counter    | `PC`                    |
| Stack pointer      | `SP`                    |
| Flags              | 16-bit                  |

The CPU has a 16-bit address space:

```text
0x0000 - 0xFFFF
```

giving a total of:

```text
65,536 bytes = 64 KiB
```

---

# Instruction Encoding

Every instruction is exactly 4 bytes:

```text
┌────────┬────────┬────────┬────────┐
│ OPCODE │  DEST  │  SRC1  │  SRC2  │
│ 8-bit  │ 8-bit  │ 8-bit  │ 8-bit  │
└────────┴────────┴────────┴────────┘
```

The fields have different meanings depending on the instruction.

For example:

```text
ADD R0, R1, R2
```

is encoded conceptually as:

```text
┌────────┬────────┬────────┬────────┐
│  ADD   │   R0   │   R1   │   R2   │
└────────┴────────┴────────┴────────┘
```

Instructions are fetched from the address contained in `PC`.

Before an instruction is executed:

```text
PC = PC + 4
```

This means control-flow instructions calculate their offsets relative to the address of the **next instruction**.

---

# Registers

The CPU contains sixteen general-purpose 16-bit registers:

```text
R0 - R15
```

All general-purpose registers are 16 bits wide.

## Special Registers

| Register | Description            |
| -------- | ---------------------- |
| `PC`     | Program Counter        |
| `SP`     | Stack Pointer          |
| `FLAGS`  | Processor status flags |

`PC`, `SP`, and `FLAGS` are separate from `R0-R15`.

## Register Conventions

The terminal interrupt handler uses:

```text
R14 = terminal input character
```

When a terminal interrupt occurs, the handler reads the character from the terminal and leaves it in `R14`.

`R14` is therefore reserved by convention for terminal input when using the current terminal interrupt handler.

---

# Initial CPU State

When the CPU is created:

```text
PC    = 0x1000
SP    = 0xFFFF
FLAGS = 0
```

Interrupts are initially enabled.

---

# Memory

Memory is byte-addressable:

```text
0x0000 - 0xFFFF
```

A byte occupies one address.

A 16-bit word occupies two consecutive addresses.

Words are stored **big-endian**.

For example:

```text
Address    Value
0x2000     high byte
0x2001     low byte
```

If the word is:

```text
0x1234
```

memory contains:

```text
0x2000 = 0x12
0x2001 = 0x34
```

---

# Memory Layout

The current memory layout is:

```text
0x0000 ───────────────── Vector table
          │
          ├── System call vectors
          │
          └── Interrupt vectors

0x0100 ───────────────── Subroutines
          │
          ├── PRINT
          ├── STRLEN
          ├── ATOI
          └── ITOA

...       ─────────────── Unused / available

0x0600 ───────────────── Interrupt handlers
          │
          └── Terminal IRQ

0x1000 ───────────────── User programs

...       ─────────────── Available RAM

0xFF00 ───────────────── Stack region
0xFFFF
```

The exact handler addresses are determined by the vector table.

The vector table contains **addresses**, rather than executable code.

---

# Instruction Set

## System

| Opcode | Instruction | Description                 |
| ------ | ----------- | --------------------------- |
| `0x00` | `NOP`       | No operation                |
| `0x01` | `HALT`      | Halt the CPU                |
| `0x02` | `SYSCALL`   | Call a vector-table routine |

---

## Data Operations

| Opcode | Instruction | Description                                 |
| ------ | ----------- | ------------------------------------------- |
| `0x10` | `MOV`       | Copy a register                             |
| `0x11` | `LOAD`      | Load a 16-bit word from memory              |
| `0x12` | `STORE`     | Store a 16-bit word to memory               |
| `0x13` | `PUSH`      | Push a register onto the stack              |
| `0x14` | `POP`       | Pop a word from the stack                   |
| `0x15` | `MOVI`      | Load a 16-bit immediate                     |
| `0x16` | `LOADB`     | Load an 8-bit byte from memory              |
| `0x17` | `STOREB`    | Store an 8-bit byte to memory               |
| `0x18` | `INC`       | Increment a register                        |
| `0x19` | `DEC`       | Decrement a register                        |
| `0x1A` | `LOADIND`   | Load a word using a register as an address  |
| `0x1B` | `STOREIND`  | Store a word using a register as an address |
| `0x1C` | `LOADBIND`  | Load a byte using a register as an address  |
| `0x1D` | `STOREBIND` | Store a byte using a register as an address |

---

## Arithmetic

| Opcode | Instruction | Description            |
| ------ | ----------- | ---------------------- |
| `0x20` | `ADD`       | Add two registers      |
| `0x21` | `SUB`       | Subtract two registers |
| `0x22` | `MUL`       | Multiply two registers |
| `0x23` | `DIV`       | Divide two registers   |

All arithmetic operates on 16-bit register values.

---

## Logic

| Opcode | Instruction | Description |
| ------ | ----------- | ----------- |
| `0x30` | `AND`       | Bitwise AND |
| `0x31` | `OR`        | Bitwise OR  |
| `0x32` | `XOR`       | Bitwise XOR |
| `0x33` | `NOT`       | Bitwise NOT |

---

## Comparison

| Opcode | Instruction | Description           |
| ------ | ----------- | --------------------- |
| `0x40` | `CMP`       | Compare two registers |

`CMP` compares `SRC1` and `SRC2`.

```text
SRC1 == SRC2 → Z
SRC1 >  SRC2 → G
SRC1 <  SRC2 → L
```

The `Z`, `G`, and `L` comparison flags are cleared before the comparison result is set.

---

# Control Flow

| Opcode | Instruction | Description                 |
| ------ | ----------- | --------------------------- |
| `0x50` | `JMP`       | Unconditional relative jump |
| `0x51` | `JE`        | Jump if equal               |
| `0x52` | `JNE`       | Jump if not equal           |
| `0x53` | `JG`        | Jump if greater             |
| `0x54` | `JL`        | Jump if less                |
| `0x55` | `CALL`      | Call a relative address     |
| `0x56` | `RET`       | Return from a subroutine    |
| `0x57` | `IRET`      | Return from an interrupt    |

## Relative Jumps

Jumps use a signed 16-bit PC-relative offset.

The CPU first increments `PC`:

```text
PC = PC + 4
```

The signed offset is then applied:

```text
PC = PC + offset
```

For example:

```asm
JMP -8
```

means:

```text
PC = next instruction - 8
```

This allows loops and branches to be encoded without storing absolute addresses.

## CALL

`CALL` performs:

```text
push current PC
PC = PC + signed offset
```

The current `PC` is already pointing to the instruction after the `CALL`.

## RET

`RET` pops the saved address:

```text
PC = pop()
```

---

# I/O

| Opcode | Instruction | Description                     |
| ------ | ----------- | ------------------------------- |
| `0x60` | `OUT`       | Write a value to an output port |
| `0x61` | `INP`       | Read a value from an input port |

The CPU provides eight input and eight output registers:

```text
INP[0-7]
OUT[0-7]
```

The register number used for I/O is supplied through a general-purpose register.

For example:

```asm
MOVI R1, 0
MOVI R2, 65
OUT R1, R2
```

writes the value `65` (`'A'`) to output port `0`.

---

# Terminal I/O

The current terminal device uses:

```text
Input port:  0
Output port: 0
```

Terminal input is handled asynchronously through the terminal interrupt.

When a character is entered:

```text
Terminal
    ↓
INP[0]
    ↓
Terminal interrupt
    ↓
Terminal IRQ handler
    ↓
R14 = character
```

The current interrupt handler also echoes the character through:

```text
OUT[0]
```

The handler then executes `IRET`.

This means the program itself decides what the character means.

For example, the program can interpret:

```text
'\n'
```

as:

```text
execute command
```

rather than the interrupt handler deciding to halt the CPU.

---

# Stack

The stack is located near the top of memory and grows downward.

Initial state:

```text
SP = 0xFFFF
```

## PUSH

`PUSH` decrements `SP` by two bytes and stores a 16-bit word:

```text
SP = SP - 2
memory[SP] = value
```

## POP

`POP` loads the word at `SP` and then increments `SP`:

```text
value = memory[SP]
SP = SP + 2
```

The stack is used by:

* `PUSH`
* `POP`
* `CALL`
* `RET`
* `SYSCALL`
* interrupts
* `IRET`

---

# Flags

`FLAGS` is a 16-bit processor status register.

|    Bit | Name     | Description  |
| -----: | -------- | ------------ |
|    `0` | `Z`      | Zero / equal |
|    `1` | `G`      | Greater than |
|    `2` | `L`      | Less than    |
|    `3` | `N`      | Negative     |
|    `4` | `V`      | Overflow     |
|    `5` | `U`      | Underflow    |
|    `6` | `H`      | Halted       |
|    `7` | `D`      | Debug        |
| `8-15` | Reserved | Unused       |

## Comparison Flags

Currently `CMP` uses:

```text
Z
G
L
```

Example:

```asm
CMP R1, R2
JE equal
JG greater
JL less
```

## Halt Flag

`HALT` sets:

```text
FLAG_H
```

The CPU execution loop terminates when this flag is set.

## Debug Flag

When:

```text
FLAG_D
```

is set, the CPU outputs debugging information for executed instructions.

## Reserved Arithmetic Flags

The following flags exist architecturally but are not currently generated by the implemented arithmetic instructions:

```text
N = Negative
V = Overflow
U = Underflow
```

They are reserved for future arithmetic/status-flag support.

---

# Interrupts

The CPU supports hardware-style interrupts.

The current implementation provides:

```text
Terminal interrupt
```

Interrupts are separate from normal `SYSCALL` execution.

A `SYSCALL` is explicitly requested by a program:

```asm
SYSCALL 0
```

An interrupt is generated by external device activity.

For example:

```text
User presses a key
        ↓
Terminal detects input
        ↓
Terminal sets interrupt pending
        ↓
CPU checks interrupts
        ↓
Terminal interrupt handler executes
```

---

# Interrupt State

The CPU maintains interrupt state internally:

```text
INTR_ENABLED
INTR_PENDING
```

`INTR_ENABLED` determines whether the CPU can service interrupts.

`INTR_PENDING` records pending interrupt requests.

The current terminal interrupt uses:

```text
INTR_TERM = 0
```

---

# Interrupt Entry

When an interrupt is pending and interrupts are enabled, the CPU:

1. Clears the pending interrupt.
2. Pushes the current `PC`.
3. Pushes `FLAGS`.
4. Disables further interrupts.
5. Looks up the interrupt handler address.
6. Jumps to the handler.

Conceptually:

```text
PC → stack
FLAGS → stack

INTR_ENABLED = 0

PC =
    memory[
        INTR_TABLE_START +
        (interrupt_number × 2)
    ]
```

The current `PC` is the address of the instruction that would have executed next.

---

# Interrupt Vector Table

The interrupt vector table is separate from the system-call vector table.

The interrupt table begins at:

```text
INTR_TABLE_START
```

Currently:

```text
INTR_TABLE_START = 0x000A
```

Each interrupt vector occupies two bytes.

The address of an interrupt vector is:

```text
INTR_TABLE_START + (interrupt_number × 2)
```

For example, terminal interrupt `0` uses:

```text
0x000A + (0 × 2)
```

giving:

```text
0x000A
```

The 16-bit value stored there is the address of the handler.

For example:

```text
0x000A → 0x0600
```

means:

```text
Terminal interrupt
        ↓
vector 0
        ↓
memory[0x000A]
        ↓
0x0600
        ↓
Terminal IRQ handler
```

The vector table therefore provides **indirection** between an interrupt number and its handler.

---

# Current Interrupt Vectors

| Interrupt | Name       | Handler      |
| --------: | ---------- | ------------ |
|       `0` | `TERMINAL` | Terminal IRQ |

The terminal vector currently points to the terminal interrupt handler located after the subroutine region.

---

# IRET

`IRET` returns from an interrupt.

Interrupt entry pushes:

```text
PC
FLAGS
```

`IRET` restores them in reverse order:

```text
FLAGS = pop()
PC    = pop()
```

It then re-enables interrupts.

Conceptually:

```text
FLAGS ← stack
PC    ← stack

INTR_ENABLED = 1
```

This allows the interrupted program to continue exactly where it left off.

---

# Interrupt Handler Convention

Interrupt handlers must preserve any general-purpose registers they modify unless a register is explicitly designated for communication with the interrupted program.

The current terminal handler uses:

```text
R14 = received character
```

Therefore the handler deliberately does not restore `R14`.

A simplified terminal interrupt looks like:

```asm
TERMINAL_IRQ:

    PUSH R1

    MOVI R1, 0
    INP R14, R1

    OUT R1, R14

    POP R1

    IRET
```

The result is:

```text
R14 = received character
```

when execution returns to the interrupted program.

---

# System Calls

System calls provide software access to routines through the vector table.

A program executes:

```asm
SYSCALL n
```

The CPU:

1. Pushes the current `PC`.
2. Calculates the vector address.
3. Loads the handler address from the vector table.
4. Jumps to the handler.

For a system call:

```text
vector address = vector number × 2
```

For example:

```asm
SYSCALL 0
```

uses:

```text
memory[0x0000]
```

as the handler address.

Unlike interrupts, system calls currently return using:

```text
RET
```

because `SYSCALL` only saves the return `PC`.

---

# System Call Vector Table

The vector table begins at:

```text
0x0000
```

Each entry is a 16-bit handler address.

| Vector | Routine  |  Address |
| -----: | -------- | -------: |
|    `0` | `PRINT`  | `0x0100` |
|    `1` | `STRLEN` | `0x0200` |
|    `2` | `ATOI`   | `0x0300` |
|    `3` | `ITOA`   | `0x0400` |

The interrupt table begins immediately after the current system-call vector entries:

```text
0x000A
```

---

# Runtime Subroutines

The CPU provides several routines through the system-call vector table.

## PRINT

```asm
SYSCALL 0
```

Prints a null-terminated string.

### Input

```text
R0 = string address
```

### Behaviour

Characters are read from memory until a null byte is encountered.

Example:

```asm
MOVI R0, MESSAGE
SYSCALL 0

MESSAGE:
    DB "Hello, world!"
```

---

## STRLEN

```asm
SYSCALL 1
```

Calculates the length of a null-terminated string.

### Input

```text
R0 = string address
```

### Output

```text
R1 = string length
```

---

## ATOI

```asm
SYSCALL 2
```

Converts an ASCII decimal string into an integer.

### Input

```text
R0 = string address
```

### Output

```text
R0 = integer
```

---

## ITOA

```asm
SYSCALL 3
```

Converts an unsigned integer into an ASCII decimal string.

### Input

```text
R0 = integer
R1 = destination buffer
```

### Output

```text
R1 = string address
```

---

## READLINE

```text
SYSCALL 4
```

Reads a line of ASCII input from the terminal into a null-terminated string.

### Input

```text
R0 = destination buffer
```

### Output

```text
R0 = address immediately after the string
```

**Behaviour**

* Reads characters supplied by the terminal interrupt handler.
* Stores characters sequentially in the destination buffer.
* ASCII `10` (`'\n'`) terminates the input.
* Appends a null terminator (`0x00`) to the string.
* The terminal interrupt handler is responsible for receiving and echoing characters.


# Instruction Syntax

## Register Operations

```asm
MOV R0, R1

ADD R0, R1, R2
SUB R0, R1, R2
MUL R0, R1, R2
DIV R0, R1, R2

AND R0, R1, R2
OR  R0, R1, R2
XOR R0, R1, R2

NOT R0, R1

INC R0
DEC R0
```

---

# Immediate Values

`MOVI` loads a full 16-bit immediate value:

```asm
MOVI R0, 1234
```

Hexadecimal values are also supported:

```asm
MOVI R0, 0x2000
```

The immediate occupies the `SRC1` and `SRC2` bytes of the instruction:

```text
MOVI R0, 0x1234

OPCODE = MOVI
DEST   = R0
SRC1   = 0x12
SRC2   = 0x34
```

---

# Memory Operations

## Direct Word Access

```asm
LOAD R0, 0x2000
```

Loads a 16-bit word from address `0x2000` into `R0`.

```asm
STORE 0x2000, R0
```

Stores the 16-bit value in `R0` at address `0x2000`.

## Direct Byte Access

```asm
LOADB R0, 0x2000
```

Loads a single byte.

```asm
STOREB 0x2000, R0
```

Stores the low 8 bits of the register.

---

# Indirect Memory Operations

Indirect operations use a register containing the memory address.

## Word

```asm
LOADIND R0, R1
```

Equivalent to:

```text
R0 = memory16[R1]
```

```asm
STOREIND R0, R1
```

Stores the value of `R1` at the address contained in `R0`.

## Byte

```asm
LOADBIND R0, R1
```

Loads a byte from the address contained in `R1`.

```asm
STOREBIND R0, R1
```

Stores the low 8 bits of `R1` at the address contained in `R0`.

These operations are useful for buffers, strings, arrays, and dynamically calculated addresses.

---

# Comparison and Branching

Example:

```asm
CMP R0, R1

JE EQUAL
JG GREATER
JL LESS
```

Labels are resolved by the assembler into signed PC-relative offsets.

Example:

```asm
LOOP:

    INC R0

    CMP R0, R1
    JL LOOP
```

---

# Assembler

The assembler supports:

* Registers `R0-R15`
* Decimal values
* Hexadecimal values
* Labels
* PC-relative jumps
* PC-relative calls
* `.ORG`
* `DB`
* Quoted strings
* Escape sequences
* Comments

Comments begin with:

```text
;
```

Example:

```asm
MOVI R0, 10    ; Load newline character
```

---

# `.ORG`

`.ORG` changes the address at which the assembler considers subsequent code to exist.

It does **not** add padding bytes to the output binary.

User programs should normally begin with:

```asm
.ORG 0x1000
```

because:

```text
PROGRAM_START = 0x1000
```

Example:

```asm
.ORG 0x1000

MOVI R0, MESSAGE
SYSCALL 0

MESSAGE:
    DB "Hello, world!"
```

The CPU begins execution at:

```text
PC = 0x1000
```

---

# `DB`

`DB` emits raw bytes.

Example:

```asm
DB 65
```

emits:

```text
0x41
```

Strings are automatically null-terminated:

```asm
DB "Hello"
```

produces:

```text
48 65 6C 6C 6F 00
```

Supported escape sequences include:

```text
\n
\t
\r
\\
\"
\0
```

Example:

```asm
MESSAGE:
    DB "Hello\n"
```

---

# Example Program

A minimal program that halts:

```asm
.ORG 0x1000

HALT
```

A simple loop:

```asm
.ORG 0x1000

LOOP:

    JMP LOOP
```

A program using a system call:

```asm
.ORG 0x1000

MOVI R0, MESSAGE
SYSCALL 0
HALT

MESSAGE:
    DB "Hello, world!"

```

---

# Example Interrupt-Driven Program

The terminal interrupt places the received character in `R14`.

A program can therefore wait for Enter without directly reading the hardware input register:

```asm
.ORG 0x1000

LOOP:

    MOVI R1, 0
    CMP R14, R1
    JE LOOP

    MOVI R1, 10
    CMP R14, R1
    JE DONE

    MOVI R14, 0

    JMP LOOP


DONE:

    HALT
```

Here the interrupt mechanism handles the hardware input, while the program decides what Enter means.

---

# CPU Execution Model

The CPU continuously performs:

```text
┌─────────────────────┐
│ Check interrupts    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Fetch instruction   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Decode instruction  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ PC += 4             │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Execute instruction │
└──────────┬──────────┘
           │
           └──────────────► repeat
```

Interrupts are checked before normal instruction execution.

When an interrupt is accepted, the interrupt handler executes before the normal instruction stream continues.

---

# Architectural Summary

```text
                 16-bit CPU
                     │
        ┌────────────┼────────────┐
        │            │            │
        ▼            ▼            ▼
    Registers      Memory         I/O
    R0-R15         64 KiB       INP / OUT
        │            │            │
        │            │            ▼
        │            │        Terminal
        │            │            │
        │            │            ▼
        │            │       Terminal IRQ
        │            │            │
        ▼            ▼            ▼
       ALU        Stack       Interrupt
        │            │         Handler
        │            │            │
        └────────────┼────────────┘
                     │
                     ▼
                    PC
```

The architecture currently provides:

* 16-bit registers and addresses
* 64 KiB byte-addressable memory
* Fixed 32-bit instructions
* Register, immediate, direct and indirect memory operations
* Arithmetic and bitwise operations
* Conditional and relative control flow
* A downward-growing stack
* Software system calls
* Vector-table-based dispatch
* Hardware-style terminal interrupts
* Interrupt save/restore with `IRET`
* A custom assembler with labels, `.ORG`, strings and raw byte emission

The architecture is designed to provide the foundations required for progressively building higher-level software, including an operating system, device drivers, runtime libraries, and user programs.
