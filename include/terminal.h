#ifndef TERMINAL_H
#define TERMINAL_H

#include "cpu.h"


void init_terminal();
void restore_terminal();
void Terminal_Update(CPU *cpu);

#endif