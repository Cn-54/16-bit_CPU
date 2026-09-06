#include "MEMORY.h"

Memory *Create_Memory(void);
void Destroy_Memory(Memory *mem);

void Load_Memory(uint16_t address,uint8_t *data);