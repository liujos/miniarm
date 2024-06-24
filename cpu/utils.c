#include "utils.h"

u32 bit(u32 sequence, u32 i)
{
    return (sequence >> i) & 1;
}

u32 bits(u32 sequence, u32 start, u32 end)
{
    return (sequence >> end) & ((1 << (start - end + 1)) - 1);
}

u32 ror(u32 sequence, u32 rotate)
{
    u32 low = sequence & ((1 << rotate) - 1);
    sequence >>= rotate;
    sequence &= ~(0x80000000 >> (rotate - 1));
    sequence |= low << (32 - rotate);
    return sequence;
}

u32 slr(u32 sequence, u32 shift)
{
    sequence >>= shift;
    sequence &= (1 << shift) - 1;
    return sequence;
}

u32 sar(u32 sequence, u32 shift)
{
    return sequence >> shift;
}

u32 rorx(u32 sequence, u32 *carry_out)
{
    u32 lsb = bit(rm, 0);
    sequence = ((sequence >> 1) & (0x7FFFFFFF)) | (*carry_out << 31);
    *carry_out = lsb;
    return sequence;
}

void dump(u32 *register_file)
{
    for (int i = 0; i < REGISTERS; i++) {
        if (i != 0 && i % 4 == 0)
            printf("\n");

        if (i < 10)
            printf("r%d:  0x%08x  ", i, register_file[i]);
        else   
            printf("r%d: 0x%08x  ", i, register_file[i]);
    }
    printf("\n\n");
}
