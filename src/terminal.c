#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>

#include "terminal.h"

#define TERMINAL_IN  0
#define TERMINAL_OUT 0

static struct termios orig_term_attr;

void init_terminal(){
    struct termios new_term_attr;

    tcgetattr(STDIN_FILENO, &orig_term_attr);

    new_term_attr = orig_term_attr;

    // Disable canonical mode and echo
    new_term_attr.c_lflag &= ~(ICANON | ECHO);

    // Non-blocking input
    new_term_attr.c_cc[VMIN] = 0;
    new_term_attr.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_term_attr);
}

void restore_terminal(){
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term_attr);
}

void Terminal_Update(CPU *cpu){
    // CPU -> terminal
    if (cpu->OUT[TERMINAL_OUT] != 0)
    {
        putchar((char)cpu->OUT[TERMINAL_OUT]);
        fflush(stdout);

        cpu->OUT[TERMINAL_OUT] = 0;
    }

    // terminal -> CPU
    if (cpu->INP[TERMINAL_IN] == 0)
    {
        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        if (select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) > 0)
        {
            unsigned char c;

            if (read(STDIN_FILENO, &c, 1) == 1)
            {
                cpu->INP[TERMINAL_IN] = c;
            }
        }
    }
}