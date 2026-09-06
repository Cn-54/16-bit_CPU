#include "CPU.H"
#include "DATA.H"
#include "MEMORY.h"

int main(){
    
    CPU *cpu = Create_CPU();
    Memory *mem = Create_Memory();

    while (!(cpu->FLAGS & FLAG_H)){
        FDE(cpu,mem);
    }
    Destroy_CPU(cpu);

    return 0;
}