#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Opcode.h"

typedef struct
{
    char *words[4];
    int quoted[4];
    int count;
} Tokens;

typedef struct
{
    const char *name;
    Opcode opcode;
} OpcodeEntry;

static const OpcodeEntry opcodeTable[] = {

    // System
    {"NOP",       NOP},
    {"HALT",      HALT},
    {"SYSCALL",   SYSCALL},

    // Data
    {"MOV",       MOV},
    {"LOAD",      LOAD},
    {"STORE",     STORE},
    {"PUSH",      PUSH},
    {"POP",       POP},
    {"MOVI",      MOVI},
    {"LOADB",     LOADB},
    {"STOREB",    STOREB},
    {"INC",       INC},
    {"DEC",       DEC},
    {"LOADIND",   LOADIND},
    {"STOREIND",  STOREIND},
    {"LOADBIND",  LOADBIND},
    {"STOREBIND", STOREBIND},

    // Arithmetic
    {"ADD",       ADD},
    {"SUB",       SUB},
    {"MUL",       MUL},
    {"DIV",       DIV},

    // Logic
    {"AND",       AND},
    {"OR",        OR},
    {"XOR",       XOR},
    {"NOT",       NOT},

    // Comparison
    {"CMP",       CMP},

    // Control flow
    {"JMP",       JMP},
    {"JE",        JE},
    {"JNE",       JNE},
    {"JG",        JG},
    {"JL",        JL},
    {"CALL",      CALL},
    {"RET",       RET},

    // I/O
    {"PUTC",      PUTC}
};


static Tokens tokenise(char line[])
{
    Tokens t = {0};
    char *p = line;

    while (*p && t.count < 4)
    {
        // Skip separators
        while (*p == ' ' || *p == ',' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        // Quoted string
        if (*p == '"')
        {
            p++;

            t.quoted[t.count] = 1;
            t.words[t.count++] = p;

            while (*p && *p != '"')
                p++;

            if (*p == '"')
            {
                *p = '\0';
                p++;
            }
        }
        else
        {
            // Normal token
            t.words[t.count++] = p;

            while (*p &&
                   *p != ' ' &&
                   *p != ',' &&
                   *p != '\t')
            {
                p++;
            }

            if (*p)
            {
                *p = '\0';
                p++;
            }

            // Uppercase normal token
            for (char *c = t.words[t.count - 1]; *c; c++)
                *c = toupper((unsigned char)*c);
        }
    }

    return t;
}


static Opcode getOpcode(const char *str)
{
    size_t count = sizeof(opcodeTable) / sizeof(opcodeTable[0]);

    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(str, opcodeTable[i].name) == 0)
            return opcodeTable[i].opcode;
    }

    return 0xFF;
}


static int getRegister(const char *str)
{
    if (str[0] != 'R')
        return -1;

    int reg = atoi(str + 1);

    if (reg < 0 || reg > 15)
        return -1;

    return reg;
}


static int getValue(const char *str)
{
    return (int)strtol(str, NULL, 0);
}


static void emit(FILE *file, int op, int dest, int src1, int src2)
{
    fputc(op, file);
    fputc(dest, file);
    fputc(src1, file);
    fputc(src2, file);
}


static int assemble(const char *input, const char *output)
{
    FILE *in = fopen(input, "r");

    if (!in)
    {
        printf("Could not open input file\n");
        return 1;
    }

    // Binary output
    FILE *out = fopen(output, "wb");

    if (!out)
    {
        printf("Could not open output file\n");
        fclose(in);
        return 1;
    }

    char line[256];

    while (fgets(line, sizeof(line), in))
    {
        // Remove newline
        line[strcspn(line, "\r\n")] = '\0';

        Tokens t = tokenise(line);

        if (t.count == 0)
            continue;

        if (strcmp(t.words[0], "DB") == 0)
        {
            if (t.count < 2)
            {
                printf("DB requires an operand\n");
                continue;
            }

            // String
            if (t.quoted[1])
            {
                char *p = t.words[1];

                while (*p)
                {
                    if (*p == '\\')
                    {
                        p++;

                        switch (*p)
                        {
                            case 'n':
                                fputc('\n', out);
                                break;

                            case 't':
                                fputc('\t', out);
                                break;

                            case 'r':
                                fputc('\r', out);
                                break;

                            case '\\':
                                fputc('\\', out);
                                break;

                            case '"':
                                fputc('"', out);
                                break;

                            case '0':
                                fputc('\0', out);
                                break;

                            default:
                                fputc('\\', out);
                                fputc(*p, out);
                                break;
                        }
                    }
                    else
                    {
                        fputc((unsigned char)*p, out);
                    }

                    p++;
                }

                // Automatically terminate every string
                fputc('\0', out);
            }

            // Single byte
            else
            {
                int value = getValue(t.words[1]);

                fputc(value & 0xFF, out);
            }

            continue;
        }


        Opcode op = getOpcode(t.words[0]);

        if (op == 0xFF)
        {
            printf("Unknown opcode: %s\n", t.words[0]);
            continue;
        }


        int dest = 0;
        int src1 = 0;
        int src2 = 0;


        switch (op)
        {
            // No operands
            case NOP:
            case HALT:
            case RET:
                break;


            // One register
            case PUSH:
            case POP:
            case INC:
            case DEC:
            case PUTC:

                dest = getRegister(t.words[1]);

                break;


            // Two registers
            case MOV:
            case LOADIND:
            case STOREIND:
            case LOADBIND:
            case STOREBIND:
            case CMP:

                dest = getRegister(t.words[1]);
                src1 = getRegister(t.words[2]);

                break;


            // Three registers
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case AND:
            case OR:
            case XOR:

                dest = getRegister(t.words[1]);
                src1 = getRegister(t.words[2]);
                src2 = getRegister(t.words[3]);

                break;


            // Register + immediate
            case MOVI:
            {
                dest = getRegister(t.words[1]);

                int value = getValue(t.words[2]);

                src1 = (value >> 8) & 0xFF;
                src2 = value & 0xFF;

                break;
            }


            // Direct memory access
            case LOAD:
            {
                dest = getRegister(t.words[1]);

                int address = getValue(t.words[2]);

                src1 = (address >> 8) & 0xFF;
                src2 = address & 0xFF;

                break;
            }


            case STORE:
            {
                int address = getValue(t.words[1]);

                dest = (address >> 8) & 0xFF;
                src1 = address & 0xFF;

                src2 = getRegister(t.words[2]);

                break;
            }


            // Byte direct memory access
            case LOADB:
            {
                dest = getRegister(t.words[1]);

                int address = getValue(t.words[2]);

                src1 = (address >> 8) & 0xFF;
                src2 = address & 0xFF;

                break;
            }


            case STOREB:
            {
                int address = getValue(t.words[1]);

                dest = (address >> 8) & 0xFF;
                src1 = address & 0xFF;

                src2 = getRegister(t.words[2]);

                break;
            }


            // Logic NOT
            case NOT:

                dest = getRegister(t.words[1]);
                src1 = getRegister(t.words[2]);

                break;


            // System call
            case SYSCALL:

                src1 = getValue(t.words[1]);

                break;


            // Relative jumps
            case JMP:
            case JE:
            case JNE:
            case JG:
            case JL:
            case CALL:
            {
                int offset = getValue(t.words[1]);

                src1 = (offset >> 8) & 0xFF;
                src2 = offset & 0xFF;

                break;
            }


            default:

                printf(
                    "Assembler does not support opcode: %s\n",
                    t.words[0]
                );

                fclose(in);
                fclose(out);

                return 1;
        }


        emit(out, op, dest, src1, src2);
    }


    fclose(in);
    fclose(out);

    return 0;
}


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("usage: ./assembler <program.A> <output.bin>\n");
        return 1;
    }

    return assemble(argv[1], argv[2]);
}