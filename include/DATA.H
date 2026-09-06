#ifndef DATA_H_
#define DATA_H_

#define WORD_SIZE      2 // 2 bytes
#define ADDRESS_SIZE   2 // 2 bytes
#define MEMORY_SIZE    (1 << 16)  // 64 KiB byte-addressable
#define MAX_STACK      256
#define REGISTER_COUNT 16

#define PAGE_SIZE 256
#define PAGE_COUNT 256

#define FLAG_Z (1 << 0) // zero flag
#define FLAG_G (1 << 1) // greater than flag
#define FLAG_L (1 << 2) // less than flag
#define FLAG_N (1 << 3) // negative flag
#define FLAG_V (1 << 4) // overflow flag
#define FLAG_U (1 << 5) // underflow flag
#define FLAG_H (1 << 6) // halted flag

#define VECTORTABLE_START 0x0000
#define SUBROUTINES_START 0x0100
#define PROGRAM_START     0x1000

#endif