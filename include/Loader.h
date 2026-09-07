#ifndef LOADER_H_
#define LOADER_H_

#include "memory.h"

void Load_Code(Memory *mem, const char *filename, uint16_t address);
void Load_Subroutines(Memory *mem, uint16_t address);
void Load_INTR_Handlers(Memory *mem, uint16_t address);

#endif