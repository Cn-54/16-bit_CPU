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

void Load_Subroutines(Memory *mem, uint16_t address){
    Load_Code(mem, "Code/subroutines/print/print.bin", address);
    Load_Code(mem, "Code/subroutines/strlen/strlen.bin", address*2);
    Load_Code(mem, "Code/subroutines/atoi/atoi.bin", address*3);
    Load_Code(mem, "Code/subroutines/itoa/itoa.bin", address*4);
    Load_Code(mem, "Code/subroutines/readline/readline.bin", address*5);
}