#ifndef EXECUTE_H
#define EXECUTE_H

#include "cpu.h"
#include "utils.h"
#include "mem_op.h"

void execute(TemporaryRegisters*, uint32_t registers[]);

enum 
{
    AND = 0b0000,
    EOR = 0b0001,
    SUB = 0b0010,
    RSB = 0b0011,
    ADD = 0b0100,
    ADC = 0b0101,
    SBC = 0b0110,
    RSC = 0b0111,
    TST = 0b1000,
    TEQ = 0b1001,
    CMP = 0b1010,
    CMN = 0b1011,
    ORR = 0b1100,
    MOV = 0b1101,
    BIC = 0b1110,
    MVN = 0b1111
};

typedef enum ShiftType 
{
    LSL = 0b00,
    LSR = 0b01,
    ASR = 0b10,
    ROR = 0b11,
} ShiftType;

enum 
{
    EQ = 0b0000,
    NE = 0b0001,
    CS = 0b0010,
    CC = 0b0011,
    MI = 0b0100,
    PL = 0b0101,
    VS = 0b0110,
    VC = 0b0111,
    HI = 0b1000,
    LS = 0b1001,
    GE = 0b1010,
    LT = 0b1011,
    GT = 0b1100,
    LE = 0b1101,
    AL = 0b1110
};

typedef struct barrelShifterParameters 
{
    uint32_t  carry;
    uint32_t  output;
    uint32_t  sequence;
    uint32_t  amount;
    ShiftType type;
} barrelShifterParameters;

#endif
