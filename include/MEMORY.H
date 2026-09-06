#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>
#include "DATA.h"

typedef struct memory {
    uint8_t data[MEMORY_SIZE];
} Memory;

Memory *Create_Memory(void);
void Destroy_Memory(Memory *mem);

void Load_Memory(uint16_t address,uint8_t *data);

#endif