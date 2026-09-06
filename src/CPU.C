#include "cpu.h"
#include "data.h"
#include "instruction.h"
#include "memory.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>

CPU *Create_CPU(void){
    CPU *cpu = calloc(1, sizeof(CPU));
    cpu->PC = PROGRAM_START;
    cpu->SP = 0xFFFF;
    cpu->FLAGS = 0;

    return cpu;
}

void Destroy_CPU(CPU *cpu){
    free(cpu);
}

static void Debug_Instruction(CPU *cpu, INS *instruction){
    printf("\n");
    printf("########################################\n");

    printf("PC    : 0x%04X\n", cpu->PC - 4);
    printf("OP    : 0x%02X\n", instruction->op);
    printf("DEST  : 0x%02X\n", instruction->DEST);
    printf("SRC1  : 0x%02X\n", instruction->SRC1);
    printf("SRC2  : 0x%02X\n", instruction->SRC2);

    printf("\nREGISTERS:\n");

    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        printf("R%-2d: 0x%04X", i, cpu->R[i]);

        if (i % 4 == 3)
            printf("\n");
        else
            printf("    ");
    }

    printf("\nSPECIAL:\n");
    printf("SP    : 0x%04X\n", cpu->SP);
    printf("FLAGS : 0x%04X\n", cpu->FLAGS);

    printf("\nFLAGS:\n");
    printf("Z=%d  G=%d  L=%d  N=%d  V=%d  U=%d  H=%d  D=%d\n",
           !!(cpu->FLAGS & FLAG_Z),
           !!(cpu->FLAGS & FLAG_G),
           !!(cpu->FLAGS & FLAG_L),
           !!(cpu->FLAGS & FLAG_N),
           !!(cpu->FLAGS & FLAG_V),
           !!(cpu->FLAGS & FLAG_U),
           !!(cpu->FLAGS & FLAG_H),
           !!(cpu->FLAGS & FLAG_D));

    printf("########################################\n");
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
        }
}