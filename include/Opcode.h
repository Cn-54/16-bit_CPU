#ifndef OPCODE_H_
#define OPCODE_H_

typedef enum {

    // System
    NOP      = 0x00,
    HALT     = 0x01,
    SYSCALL  = 0x02,

    // Data operations
    MOV      = 0x10,
    LOAD     = 0x11,
    STORE    = 0x12,
    PUSH     = 0x13,
    POP      = 0x14,

    // Arithmetic
    ADD      = 0x20,
    SUB      = 0x21,
    MUL      = 0x22,
    DIV      = 0x23,

    // Logic
    AND      = 0x30,
    OR       = 0x31,
    XOR      = 0x32,
    NOT      = 0x33,

    // Comparison
    CMP      = 0x40,

    // Control flow
    JMP      = 0x50,
    JE       = 0x51,
    JNE      = 0x52,
    JG       = 0x53,
    JL       = 0x54,
    CALL     = 0x55,
    RET      = 0x56

} Opcode;

#endif