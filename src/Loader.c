#include "loader.h"
#include <stdio.h>

void Load_Code(Memory *mem, const char *filename, uint16_t address){
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        return;
    }

    uint8_t byte;

    while (fread(&byte, 1, 1, file) == 1) {
        store_byte(mem, address, byte);
        address++;
    }

    fclose(file);
}