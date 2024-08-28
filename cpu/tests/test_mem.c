#include "mem.h"

int test_load8()
{
    u32 bytes = 256;
    u8 mem[bytes];
    for (int i = 0; i < bytes; i++)
        mem[i] = i;

    if (load8(mem, 33) != 33) {
        printf("hello\n");
        return 1;
    }

    return 0;
}

int test_load32()
{
    u32 bytes = 256;
    u8 mem[bytes];
    for (int i = 0; i < bytes; i++)
        mem[i] = i;

    if (load32(mem, 4) != (4 | 5 << 8 | 6 << 16 | 7 << 24)) 
        return 1;
    
    if (load32(mem, 5) != (5 | 6 << 8 | 7 << 16 | 4 << 24)) 
        return 1;

    if (load32(mem, 6) != (6 | 7 << 8 | 4 << 16 | 5 << 24))
        return 1;

    if (load32(mem, 7) != (7 | 4 << 8 | 5 << 16 | 6 << 24))
        return 1;

    return 0;
}

int test_store8()
{
    u32 bytes = 256;
    u8 mem[bytes];
    for (int i = 0; i < bytes; i++)
        mem[i] = i;

    store8(mem, 233, 45);
    if (load8(mem, 233) != 45)
        return 1;
    return 0;
}

int test_store32()
{
    u32 bytes = 256;
    u8 mem[bytes];
    for (int i = 0; i < bytes; i++)
        mem[i] = i;

    store32(mem, 100, 0x12345678);
    if (load32(mem, 100) != 0x12345678)
        return 1;

    return 0;
}

int main()
{
    int cnt = 4;
    int outputs[cnt];
    outputs[0] = test_load8();
    outputs[1] = test_load32();
    outputs[2] = test_store8();
    outputs[3] = test_store32();

    for (int i = 0; i < cnt; i++) {
        printf("test %d..  ", i);
        if (outputs[i] != 0)
            printf("failed\n");
        else
            printf("ok\n");
    }

    return 0;
}
