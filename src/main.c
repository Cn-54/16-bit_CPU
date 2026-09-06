#include "DATA.h"
#include "cpu.h"
#include "data.h"
#include "loader.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
    int debug = 0;
    char *program = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-D") == 0)
        {
            debug = 1;
        }
        else if (program == NULL)
        {
            program = argv[i];
        }
        else
        {
            printf("Usage: %s [-D] <program.bin>\n", argv[0]);
            return 1;
        }
    }

    if (program == NULL)
    {
        printf("Usage: %s [-D] <program.bin>\n", argv[0]);
        return 1;
    }

    CPU *cpu = Create_CPU();
    Memory *mem = Create_Memory();

    // load vector table into memory
    Load_Code(mem, "Code/vector_table/vec_table.bin", VECTORTABLE_START);

    // load subroutines into memory
    Load_Subroutines(mem,SUBROUTINES_START);
    
    // load user program into memory
    Load_Code(mem,argv[1],PROGRAM_START);

     if (debug){
        cpu->FLAGS |= FLAG_D;
    }

    while (!(cpu->FLAGS & FLAG_H)){
        FDE(cpu,mem);
    }
    Destroy_CPU(cpu);
    Destroy_Memory(mem);

    return 0;
}