#include "MEMORY.h"

Memory *Create_Memory(void);

void Destroy_Memory(Memory *mem);

uint8_t load_byte(Memory *mem, uint16_t address);
uint16_t load_word(Memory *mem, uint16_t address);

void store_byte(Memory *mem, uint16_t address, uint8_t value);
void store_word(Memory *mem, uint16_t address, uint16_t value);