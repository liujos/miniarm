#include "utils.h"

uint32_t 
changeBit
(
    uint32_t sequence, 
    uint32_t index, 
    bool     status
)
{
    return (sequence & ~(1 << index)) | (status << index);
}

uint32_t 
logicalShiftLeft
(
    uint32_t sequence, 
    uint32_t amount
)
{
    if (amount >= 32)
    {
        return 0;
    }

    return sequence << amount;
}

uint32_t
logicalShiftRight
(
    uint32_t sequence, 
    uint32_t amount
)
{
    if (amount >= 32)
    {
        return 0;
    }

    return sequence >> amount;
}

uint32_t 
arithmeticShiftRight
(
    uint32_t sequence,
    uint32_t amount
)
{
    if (amount >= 32)
    {
        amount = 31;
    }

    int32_t msb = 0x80000000 & sequence;
    return (sequence >> amount) | (msb >> amount);
}

uint32_t 
rotateRight
(
    uint32_t sequence,
    uint32_t amount
)
{
    if (amount == 32)
    {
        return sequence;
    }

    return (sequence >> amount) | (sequence << (32 - amount));
}

uint32_t 
rotateRightExtended
(
    uint32_t sequence, 
    uint32_t carry
)
{
    return (sequence >> 1) | (carry << 31);
}

uint32_t 
bit
(
    uint32_t sequence, 
    uint32_t index
)
{
    return (sequence >> index) & 1;
}

uint32_t 
bits
(
    uint32_t sequence, 
    uint32_t start, 
    uint32_t end
)
{
    return (sequence >> end) & ((1 << (start - end + 1)) - 1);
}

void 
dump
(
    uint32_t registers[]
)
{
    for (int i = 0; i < 17; i++) 
    {
        if (i != 0 && i % 4 == 0)
        {
            printf("\n");
        }

        if (i < 10)
        {
            printf("r%d:  0x%08x  ", i, registers[i]);
        }
        else   
        {
            printf("r%d: 0x%08x  ", i, registers[i]);
        }
    }

    printf("\n\n");
}
