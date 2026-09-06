#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#include "DATA.h"
#include "Opcode.h"


#define MAX_LABELS 256
#define MAX_LABEL_LENGTH 64


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


typedef struct
{
    char name[MAX_LABEL_LENGTH];
    int address;
} Label;



static const OpcodeEntry opcodeTable[] = {

    /* System */
    {"NOP",       NOP},
    {"HALT",      HALT},
    {"SYSCALL",   SYSCALL},

    /* Data */
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

    /* Arithmetic */
    {"ADD",       ADD},
    {"SUB",       SUB},
    {"MUL",       MUL},
    {"DIV",       DIV},

    /* Logic */
    {"AND",       AND},
    {"OR",        OR},
    {"XOR",       XOR},
    {"NOT",       NOT},

    /* Comparison */
    {"CMP",       CMP},

    /* Control flow */
    {"JMP",       JMP},
    {"JE",        JE},
    {"JNE",       JNE},
    {"JG",        JG},
    {"JL",        JL},
    {"CALL",      CALL},
    {"RET",       RET},

    /* I/O */
    {"PUTC",      PUTC}
};


static Label labels[MAX_LABELS];
static int labelCount = 0;



static void trim(char *str)
{
    char *start = str;

    while (isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);

    size_t len = strlen(str);

    while (len > 0 && isspace((unsigned char)str[len - 1]))
    {
        str[len - 1] = '\0';
        len--;
    }
}

static void remove_comment(char *line)
{
    int quoted = 0;

    for (char *p = line; *p; p++)
    {
        if (*p == '"' && (p == line || p[-1] != '\\'))
            quoted = !quoted;

        if (*p == ';' && !quoted)
        {
            *p = '\0';
            break;
        }
    }
}


/* ---------------------------------------------------------
 * Tokenisation
 * --------------------------------------------------------- */

static Tokens tokenise(char line[])
{
    Tokens t = {0};
    char *p = line;

    while (*p && t.count < 4)
    {
        /* Skip separators */
        while (*p == ' ' || *p == ',' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        /* Quoted string */
        if (*p == '"')
        {
            p++;

            t.quoted[t.count] = 1;
            t.words[t.count++] = p;

            while (*p)
            {
                if (*p == '"' && (p == line || p[-1] != '\\'))
                    break;

                p++;
            }

            if (*p == '"')
            {
                *p = '\0';
                p++;
            }
        }
        else
        {
            /* Normal token */
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

            /* Uppercase normal token */
            for (char *c = t.words[t.count - 1]; *c; c++)
                *c = toupper((unsigned char)*c);
        }
    }

    return t;
}

static Opcode getOpcode(const char *str)
{
    size_t count =
        sizeof(opcodeTable) / sizeof(opcodeTable[0]);

    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(str, opcodeTable[i].name) == 0)
            return opcodeTable[i].opcode;
    }

    return 0xFF;
}


static int getRegister(const char *str)
{
    if (str == NULL || str[0] != 'R')
        return -1;

    char *end;

    long reg = strtol(str + 1, &end, 10);

    if (*end != '\0')
        return -1;

    if (reg < 0 || reg > 15)
        return -1;

    return (int)reg;
}


static int getValue(const char *str)
{
    return (int)strtol(str, NULL, 0);
}


static int findLabel(const char *name)
{
    for (int i = 0; i < labelCount; i++)
    {
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].address;
    }

    return -1;
}

static int addLabel(const char *name, int address)
{
    if (labelCount >= MAX_LABELS)
    {
        printf("Too many labels\n");
        return 0;
    }

    if (strlen(name) >= MAX_LABEL_LENGTH)
    {
        printf("Label too long: %s\n", name);
        return 0;
    }

    if (findLabel(name) != -1)
    {
        printf("Duplicate label: %s\n", name);
        return 0;
    }

    strcpy(labels[labelCount].name, name);
    labels[labelCount].address = address;

    labelCount++;

    return 1;
}


static int getLineSize(Tokens *t)
{
    if (t->count == 0)
        return 0;

    /* DB */
    if (strcmp(t->words[0], "DB") == 0)
    {
        if (t->count < 2)
            return 0;

        if (t->quoted[1])
        {
            int size = 0;
            char *p = t->words[1];

            while (*p)
            {
                if (*p == '\\')
                {
                    p++;

                    if (*p)
                        p++;
                    else
                        break;
                }
                else
                {
                    p++;
                }

                size++;
            }

            return size + 1;
        }

        return 1;
    }

    return 4;
}



static void emit(FILE *file,
                 int op,
                 int dest,
                 int src1,
                 int src2)
{
    fputc(op, file);
    fputc(dest, file);
    fputc(src1, file);
    fputc(src2, file);
}



static int resolveValue(const char *str)
{
    char *end;

    long value = strtol(str, &end, 0);

    if (*end == '\0')
        return (int)value;

    int address = findLabel(str);

    if (address == -1)
        return -1;

    return address;
}


static int firstPass(const char *input)
{
    FILE *in = fopen(input, "r");

    if (!in)
    {
        printf("Could not open input file\n");
        return 0;
    }

    char line[256];

    int address = PROGRAM_START;

    while (fgets(line, sizeof(line), in))
    {
        line[strcspn(line, "\r\n")] = '\0';

        remove_comment(line);
        trim(line);

        if (line[0] == '\0')
            continue;


        char *colon = strchr(line, ':');

        if (colon)
        {
            *colon = '\0';

            trim(line);

            if (line[0] == '\0')
            {
                printf("Empty label\n");
                fclose(in);
                return 0;
            }


            for (char *c = line; *c; c++)
                *c = toupper((unsigned char)*c);

            if (!addLabel(line, address))
            {
                fclose(in);
                return 0;
            }


            char *instruction = colon + 1;

            trim(instruction);

            if (*instruction == '\0')
                continue;

            strcpy(line, instruction);
        }

        Tokens t = tokenise(line);

        address += getLineSize(&t);
    }

    fclose(in);

    return 1;
}


static void emitDB(FILE *out, Tokens *t)
{
    if (t->count < 2)
    {
        printf("DB requires an operand\n");
        return;
    }

    if (t->quoted[1])
    {
        char *p = t->words[1];

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

        fputc('\0', out);
    }
    else
    {
        int value = getValue(t->words[1]);

        fputc(value & 0xFF, out);
    }
}


static int secondPass(const char *input,
                      const char *output)
{
    FILE *in = fopen(input, "r");

    if (!in)
    {
        printf("Could not open input file\n");
        return 0;
    }

    FILE *out = fopen(output, "wb");

    if (!out)
    {
        printf("Could not open output file\n");
        fclose(in);
        return 0;
    }

    char line[256];

    int address = 0;

    while (fgets(line, sizeof(line), in))
    {
        line[strcspn(line, "\r\n")] = '\0';

        remove_comment(line);
        trim(line);

        if (line[0] == '\0')
            continue;

        char *colon = strchr(line, ':');

        if (colon)
        {
            char *instruction = colon + 1;

            trim(instruction);

            if (*instruction == '\0')
                continue;

            strcpy(line, instruction);
        }

        Tokens t = tokenise(line);

        if (t.count == 0)
            continue;

        if (strcmp(t.words[0], "DB") == 0)
        {
            emitDB(out, &t);

            address += getLineSize(&t);

            continue;
        }


        Opcode op = getOpcode(t.words[0]);

        if (op == 0xFF)
        {
            printf(
                "Unknown opcode at 0x%04X: %s\n",
                address,
                t.words[0]
            );

            fclose(in);
            fclose(out);

            return 0;
        }


        int dest = 0;
        int src1 = 0;
        int src2 = 0;


        switch (op)
        {
            /* No operands */

            case NOP:
            case HALT:
            case RET:
                break;


            /* One destination register */

            case INC:
            case DEC:

                dest = getRegister(t.words[1]);

                if (dest < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;


            /* One source register */

            case PUSH:
            case POP:
            case PUTC:

                src1 = getRegister(t.words[1]);

                if (src1 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;


            /* Two registers */

            case MOV:
            case LOADIND:
            case STOREIND:
            case LOADBIND:
            case STOREBIND:

                dest = getRegister(t.words[1]);
                src1 = getRegister(t.words[2]);

                if (dest < 0 || src1 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;


            case CMP:

                src1 = getRegister(t.words[1]);
                src2 = getRegister(t.words[2]);

                if (src1 < 0 || src2 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;


            /* Three registers */

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

                if (dest < 0 || src1 < 0 || src2 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;


            /* MOVI */

            case MOVI:
            {
                dest = getRegister(t.words[1]);

                if (dest < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                int value = resolveValue(t.words[2]);

                if (value < 0)
                {
                    printf(
                        "Unknown value: %s\n",
                        t.words[2]
                    );

                    goto error;
                }

                src1 = (value >> 8) & 0xFF;
                src2 = value & 0xFF;

                break;
            }


            /* Direct LOAD */

            case LOAD:
            {
                dest = getRegister(t.words[1]);

                if (dest < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                int addressValue =
                    resolveValue(t.words[2]);

                if (addressValue < 0)
                {
                    printf(
                        "Unknown address: %s\n",
                        t.words[2]
                    );

                    goto error;
                }

                src1 = (addressValue >> 8) & 0xFF;
                src2 = addressValue & 0xFF;

                break;
            }


            /* Direct STORE */

            case STORE:
            {
                int addressValue =
                    resolveValue(t.words[1]);

                if (addressValue < 0)
                {
                    printf(
                        "Unknown address: %s\n",
                        t.words[1]
                    );

                    goto error;
                }

                dest = (addressValue >> 8) & 0xFF;
                src1 = addressValue & 0xFF;

                src2 = getRegister(t.words[2]);

                if (src2 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;
            }


            /* Direct LOADB */

            case LOADB:
            {
                dest = getRegister(t.words[1]);

                if (dest < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                int addressValue =
                    resolveValue(t.words[2]);

                if (addressValue < 0)
                {
                    printf(
                        "Unknown address: %s\n",
                        t.words[2]
                    );

                    goto error;
                }

                src1 = (addressValue >> 8) & 0xFF;
                src2 = addressValue & 0xFF;

                break;
            }


            /* Direct STOREB */

            case STOREB:
            {
                int addressValue =
                    resolveValue(t.words[1]);

                if (addressValue < 0)
                {
                    printf(
                        "Unknown address: %s\n",
                        t.words[1]
                    );

                    goto error;
                }

                dest = (addressValue >> 8) & 0xFF;
                src1 = addressValue & 0xFF;

                src2 = getRegister(t.words[2]);

                if (src2 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;
            }


            /* NOT */

            case NOT:

                dest = getRegister(t.words[1]);
                src1 = getRegister(t.words[2]);

                if (dest < 0 || src1 < 0)
                {
                    printf("Invalid register\n");
                    goto error;
                }

                break;


            /* SYSCALL */

            case SYSCALL:

                src1 = resolveValue(t.words[1]);

                if (src1 < 0)
                {
                    printf(
                        "Unknown syscall: %s\n",
                        t.words[1]
                    );

                    goto error;
                }

                break;


            case JMP:
            case JE:
            case JNE:
            case JG:
            case JL:
            case CALL:
            {
                int target = resolveValue(t.words[1]);

                if (target < 0)
                {
                    printf(
                        "Unknown label: %s\n",
                        t.words[1]
                    );

                    goto error;
                }

                int offset =
                    target - (address + 4);

                if (offset < -32768 || offset > 32767)
                {
                    printf(
                        "Jump offset out of range at 0x%04X\n",
                        address
                    );

                    goto error;
                }

                src1 = (offset >> 8) & 0xFF;
                src2 = offset & 0xFF;

                break;
            }


            default:

                printf(
                    "Assembler does not support opcode: %s\n",
                    t.words[0]
                );

                goto error;
        }


        emit(out, op, dest, src1, src2);

        address += 4;

        continue;


    error:

        fclose(in);
        fclose(out);

        return 0;
    }

    fclose(in);
    fclose(out);

    return 1;
}


static int assemble(const char *input,
                    const char *output)
{
    labelCount = 0;

    if (!firstPass(input))
        return 1;


    if (!secondPass(input, output))
        return 1;


    return 0;
}


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf(
            "usage: ./assembler <program.A> <output.bin>\n"
        );

        return 1;
    }

    return assemble(argv[1], argv[2]);
}
