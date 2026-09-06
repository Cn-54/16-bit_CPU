#include "CPU.H"
#include <cstdlib>
#include <stdlib.h>

CPU *Create_CPU(void){
    return calloc(1, sizeof(CPU));
}

void Destroy_CPU(CPU *cpu){
    free(cpu);
}