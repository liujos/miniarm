#ifndef CPU_H
#define CPU_H
#include "utils.h"

#define REGISTERS 16
#define PC 15
#define LR 14
#define N 31
#define Z 30
#define C 29
#define V 28
#define CPSR 16
#define MEMORY_SIZE 0x1000

typedef struct TemporaryRegisters 
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t instruction;
    uint32_t loadMemoryData;
    uint32_t ALUOutput;
    uint32_t singleDataTransferOffset;
    uint32_t operation;
    uint32_t condition;
    bool     writeback;
    bool     link;
} TemporaryRegisters;

enum 
{
    DATA = 0,
    MUL = 0x90,
    LDR = 0x04100000,
    LDRB = 0x04500000,
    STR = 0x04000000,
    STRB = 0x04400000,
    BRANCH = 0x0A000000
};

enum 
{
    MULT_MASK = 0x0FC000F0,
    DATA_MASK = 0x0C000000,
    BRANCH_MASK = 0x0E000000,
    SDT_MASK = 0x0C500000
};

#endif
