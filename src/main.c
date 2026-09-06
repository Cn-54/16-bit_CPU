#include "DATA.h"
#include "cpu.h"
#include "data.h"
#include "loader.h"
#include "memory.h"

int main(){
    
    CPU *cpu = Create_CPU();
    Memory *mem = Create_Memory();

    Load_Code(mem, "Code/vector_table/vec_table.bin", VECTORTABLE_START);
    Load_Code(mem, "Code/subroutines/print/print.bin", SUBROUTINES_START);
    Load_Code(mem, "Code/programs/program.bin", PROGRAM_START);

    cpu->FLAGS |= FLAG_D;

    while (!(cpu->FLAGS & FLAG_H)){
        FDE(cpu,mem);
    }
    Destroy_CPU(cpu);
    Destroy_Memory(mem);

    return 0;
}