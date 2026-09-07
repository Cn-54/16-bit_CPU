#include <stdio.h>
#include <stdint.h>
#include "terminal.h"

#define TERMINAL_IN  0
#define TERMINAL_OUT 0

void Terminal_Update(CPU *cpu)
{
    // cpu to terminal
    if (cpu->OUT[TERMINAL_OUT] != 0)
    {
        putchar((char)cpu->OUT[TERMINAL_OUT]);
        fflush(stdout);

        cpu->OUT[TERMINAL_OUT] = 0;
    }

    // terminal to cpu
    if (cpu->OUT[TERMINAL_IN] == 0)
    {
        int c = getchar();

        if (c != EOF)
            cpu->INP[TERMINAL_IN] = (uint16_t)c;
    }
}