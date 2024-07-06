#include <stdint.h>
#include <stdio.h>

#ifndef UTILS_H
#define UTILS_H

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t i32;

u32 lsl(u32 sequence, u32 amount)
{
    if (amount >= 32)
        return 0;
    return sequence << amount;
}

u32 lsr(u32 sequence, u32 amount)
{
    if (amount >= 32)
        return 0;
    return sequence >> amount;
}

u32 asr(u32 sequence, u32 amount)
{
    if (amount >= 32)
        amount = 31;

    i32 msb = 0x80000000 & sequence;
    return (sequence >> amount) | (msb >> amount);
}

u32 ror(u32 sequence, u32 amount)
{
    if (amount == 32)
        return sequence;
    return (sequence >> amount) | (sequence << (32 - amount));
}

u32 rrx(u32 sequence, u32 carry)
{
    return (sequence >> 1) | (carry << 31);
}

u32 bit(u32 sequence, u32 i)
{
    return (sequence >> i) & 1;
}

u32 bits(u32 sequence, u32 start, u32 end)
{
    return (sequence >> end) & ((1 << (start - end + 1)) - 1);
}

void dump(u32 *register_file)
{
    for (int i = 0; i < 16; i++) {
        if (i != 0 && i % 4 == 0)
            printf("\n");

        if (i < 10)
            printf("r%d:  0x%08x  ", i, register_file[i]);
        else   
            printf("r%d: 0x%08x  ", i, register_file[i]);
    }
    printf("\n\n");
}

#endif
