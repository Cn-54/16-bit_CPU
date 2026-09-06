#include "cpu.h"
#include "data.h"
#include "instruction.h"
#include "memory.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>

CPU *Create_CPU(void){
    CPU *cpu = calloc(1, sizeof(CPU));
    if(cpu == NULL){
        printf(" [!] error creating cpu");
        return NULL;
    }
    cpu->PC = PROGRAM_START;
    cpu->SP = 0xFFFF;
    cpu->FLAGS = 0;

    return cpu;
}

void Destroy_CPU(CPU *cpu){
    free(cpu);
}

static void Debug_Instruction(CPU *cpu, INS *instruction){
    printf(
        "[%04X] %02X %02X,%02X,%02X | "
        "R0=%04X R1=%04X R2=%04X R3=%04X | "
        "SP=%04X | FLAGS=%04X [",
        
        cpu->PC,
        instruction->op,
        instruction->DEST,
        instruction->SRC1,
        instruction->SRC2,

        cpu->R[0],
        cpu->R[1],
        cpu->R[2],
        cpu->R[3],

        cpu->SP,
        cpu->FLAGS
    );

    if (cpu->FLAGS & FLAG_Z) printf("Z");
    if (cpu->FLAGS & FLAG_G) printf("G");
    if (cpu->FLAGS & FLAG_L) printf("L");
    if (cpu->FLAGS & FLAG_N) printf("N");
    if (cpu->FLAGS & FLAG_V) printf("V");
    if (cpu->FLAGS & FLAG_U) printf("U");
    if (cpu->FLAGS & FLAG_H) printf("H");
    if (cpu->FLAGS & FLAG_D) printf("D");

    printf("]\n");
}

void FDE(CPU *cpu, Memory *mem){
    INS instruction;

    instruction.op   = (Opcode) mem->data[cpu->PC];
    instruction.DEST =          mem->data[cpu->PC + 1];
    instruction.SRC1 =          mem->data[cpu->PC + 2];
    instruction.SRC2 =          mem->data[cpu->PC + 3];

    uint16_t addressA = instruction.DEST << 8 | instruction.SRC1;
    uint16_t addressB = instruction.SRC1 << 8 | instruction.SRC2;

    if (cpu->FLAGS & FLAG_D) {
        Debug_Instruction(cpu, &instruction);
    }

    cpu->PC += 4;

    if(cpu->PC > MEMORY_SIZE - MAX_STACK){
        printf(" [!] CPU ERROR: Memory limits exceeded");
        cpu->FLAGS |= FLAG_H;
    }

    switch (instruction.op) {
        case (NOP):
            break;
        case HALT:
            cpu->FLAGS |= FLAG_H;
            break;
        case SYSCALL: {
            cpu->SP -= WORD_SIZE;

            uint16_t return_address = cpu->PC;

            store_word(mem, cpu->SP, return_address);

            uint16_t vector_address = instruction.SRC1 * WORD_SIZE;
            uint16_t handler = load_word(mem, vector_address);

            cpu->PC = handler;
            break;
        }
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
        case LOADB:
            cpu->R[instruction.DEST] = load_byte(mem, addressB);
            break;
        case STOREB:
            store_byte(mem, addressA, cpu->R[instruction.SRC2]);
            break;
        case INC:
            cpu->R[instruction.DEST] ++;
            break;
        case DEC:
            cpu->R[instruction.DEST] --;
            break;
        case PUTC:

            putchar((char)cpu->R[instruction.SRC1]);
            fflush(stdout);
            break;

        case LOADIND:
            cpu->R[instruction.DEST] =
                load_word(mem, cpu->R[instruction.SRC1]);
            break;

        case STOREIND:
            store_word(
                mem,
                cpu->R[instruction.DEST],
                cpu->R[instruction.SRC1]
            );
            break;

        case LOADBIND:
            cpu->R[instruction.DEST] =
                load_byte(mem, cpu->R[instruction.SRC1]);

            break;

        case STOREBIND:
            store_byte(
                mem,
                cpu->R[instruction.DEST],
                (uint8_t)cpu->R[instruction.SRC1]
            );
            break;
        case MOVI:
            cpu->R[instruction.DEST] =
                ((uint16_t)instruction.SRC1 << 8) |
                instruction.SRC2;
            break;
        case INP:
            cpu->R[instruction.SRC1] = (uint16_t)getchar();
            break;
        default:
            printf(" [!] CPU ERROR: UNKNOWN OPCODE %d",instruction.op);
            cpu->FLAGS |= FLAG_H;
            break;
        }

}