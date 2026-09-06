#include "DATA.h"
#include "cpu.h"
#include "data.h"
#include "loader.h"
#include "memory.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <program.bin>\n", argv[0]);
        return 1;
    }
    CPU *cpu = Create_CPU();
    Memory *mem = Create_Memory();

    // load vector table and subroutines into memory
    Load_Code(mem, "Code/vector_table/vec_table.bin", VECTORTABLE_START);
    Load_Code(mem, "Code/subroutines/print/print.bin", SUBROUTINES_START);
    Load_Code(mem, "Code/subroutines/strlen/strlen.bin", SUBROUTINES_START*2);
    Load_Code(mem, "Code/subroutines/atoi/atoi.bin", SUBROUTINES_START*3);
    Load_Code(mem, "Code/subroutines/itoa/itoa.bin", SUBROUTINES_START*4);
    
    // load user program into memory
    Load_Code(mem,argv[1],PROGRAM_START);

    // cpu->FLAGS |= FLAG_D;

    while (!(cpu->FLAGS & FLAG_H)){
        FDE(cpu,mem);
    }
    Destroy_CPU(cpu);
    Destroy_Memory(mem);

    return 0;
}