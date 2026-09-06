#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "data.h"
#include "memory.h"

typedef struct cpu {
    uint16_t R[REGISTER_COUNT]; // R0-R15
    uint16_t FLAGS;             // Status flags
    uint16_t SP;                // Stack pointer
    uint16_t PC;                // Program counter
} CPU;

CPU *Create_CPU(void);
void Destroy_CPU(CPU *cpu);

void FDE(CPU *cpu,Memory *mem);

#endif