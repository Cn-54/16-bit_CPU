#include "CPU.H"
#include "DATA.H"

int main(){
    CPU *cpu = Create_CPU();
    while (!(cpu->FLAGS & FLAG_H)){
        FDE(cpu);
    }
    Destroy_CPU(cpu);

    return 0;
}