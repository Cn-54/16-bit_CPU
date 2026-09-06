#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "opcode.h"
#include <stdint.h>

typedef struct {
    Opcode op;
    uint8_t DEST;
    uint8_t SRC1;
    uint8_t SRC2;
} INS;
    

#endif