#include "memory.h"

#include "memory.h"
#include <stdlib.h>

Memory *Create_Memory(void)
{
    return calloc(1, sizeof(Memory));
}

void Destroy_Memory(Memory *mem)
{
    free(mem);
}

uint8_t load_byte(Memory *mem, uint16_t address)
{
    return mem->data[address];
}

uint16_t load_word(Memory *mem, uint16_t address)
{
    return ((uint16_t)mem->data[address] << 8)
         | mem->data[address + 1];
}

void store_byte(Memory *mem, uint16_t address, uint8_t value)
{
    mem->data[address] = value;
}

void store_word(Memory *mem, uint16_t address, uint16_t value)
{
    mem->data[address]     = (uint8_t)(value >> 8);
    mem->data[address + 1] = (uint8_t)(value & 0xFF);
}