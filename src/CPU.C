#include "CPU.h"
#include "DATA.h"
#include "Instruction.h"
#include "MEMORY.h"
#include "Opcode.h"
#include <stdlib.h>

CPU *Create_CPU(void){
    return calloc(1, sizeof(CPU));
}

void Destroy_CPU(CPU *cpu){
    free(cpu);
}

uint16_t load_word(Memory *mem, uint16_t address)
{
    return ((uint16_t)mem->data[address] << 8) |
           mem->data[address + 1];
}

void store_word(Memory *mem, uint16_t address, uint16_t value)
{
    mem->data[address]     = value >> 8;
    mem->data[address + 1] = value & 0xFF;
}

void FDE(CPU *cpu, Memory *mem){
    INS instruction;

    instruction.op   = (Opcode) mem->data[cpu->PC];
    instruction.DEST =          mem->data[cpu->PC + 1];
    instruction.SRC1 =          mem->data[cpu->PC + 2];
    instruction.SRC2 =          mem->data[cpu->PC + 3];

    uint16_t addressA = instruction.DEST << 8 | instruction.SRC1;
    uint16_t addressB = instruction.SRC1 << 8 | instruction.SRC2;

    cpu->PC += 4;

    switch (instruction.op) {
        case (NOP):
            break;
        case HALT:
            cpu->FLAGS |= FLAG_H;
            break;
        case SYSCALL:
            cpu->SP -= WORD_SIZE;

            uint16_t return_address = cpu->PC;

            mem->data[cpu->SP]     = return_address >> 8;
            mem->data[cpu->SP + 1] = return_address & 0xFF;

            uint16_t vector_address = instruction.SRC1 * WORD_SIZE;

            cpu->PC = load_word(mem, vector_address);
            break;
        case MOV:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1];
            break;
        case LOAD:
            cpu->R[instruction.DEST] = load_word(mem, addressB);
            break;
        case STORE:
            store_word(mem, addressA, cpu->R[instruction.SRC2]);
            break;
        case PUSH:
            cpu->SP -= 2;
            store_word(mem, cpu->SP, cpu->R[instruction.SRC1]);
            break;
        case POP:
            cpu->R[instruction.SRC1] = load_word(mem, cpu->SP);
            cpu->SP += 2;
            break;
        case ADD:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] + cpu->R[instruction.SRC2];
            break;
        case SUB:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] - cpu->R[instruction.SRC2];
            break;
        case MUL:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] * cpu->R[instruction.SRC2];
            break;
        case DIV:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] / cpu->R[instruction.SRC2];
            break;
        case AND:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] & cpu->R[instruction.SRC2];
            break;
        case OR:
        cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] | cpu->R[instruction.SRC2];
            break;
        case XOR:
            cpu->R[instruction.DEST] = cpu->R[instruction.SRC1] ^ cpu->R[instruction.SRC2];
            break;
        case NOT:
            cpu->R[instruction.DEST] = ~cpu->R[instruction.SRC1];
            break;
        case CMP:{
            cpu->FLAGS &= ~(FLAG_Z | FLAG_G | FLAG_L);

            uint16_t valA = cpu->R[instruction.SRC1];
            uint16_t valB = cpu->R[instruction.SRC2];
            if(valA > valB){
                cpu->FLAGS |= FLAG_G;
            }else if(valA < valB){
                cpu->FLAGS |= FLAG_L;
            }else{
                cpu->FLAGS |= FLAG_Z;
            }
            break;
        }
        case JMP:
            cpu->PC +=  (int16_t)addressB;
            break;
        case JE:{
            if(cpu->FLAGS & FLAG_Z){
                cpu->PC +=  (int16_t)addressB;
            }
            break;
        }
        case JNE:{
            if(!(cpu->FLAGS & FLAG_Z)){
                cpu->PC +=  (int16_t)addressB;
            }
            break;
        }
        case JG:{
            if(cpu->FLAGS & FLAG_G){
                cpu->PC +=  (int16_t)addressB;
            }
            break;
        }
        case JL:{
            if(cpu->FLAGS & FLAG_L){
                cpu->PC +=  (int16_t)addressB;
            }
            break;
        }
        case CALL:{

            cpu->SP -= WORD_SIZE;

            store_word(mem, cpu->SP, cpu->PC);

            cpu->PC +=  (int16_t)addressB;

            break;
        }

        case RET:{
            uint16_t return_address = load_word(mem, cpu->SP);

            cpu->SP += WORD_SIZE;
            cpu->PC = return_address;

            break;
        }
    }
}