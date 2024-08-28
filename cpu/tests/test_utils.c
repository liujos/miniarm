#include "utils.h"

int test_lsl(void)
{
    if (lsl(0xF0F0F0F0, 4) != 0x0F0F0F00)
        return 1;

    if (lsl(0xFFFFFFFF, 32) != 0)
        return 1;

    return 0;
}

int test_lsr(void)
{
    if (lsr(0xF0F0F0F0, 4) != 0x0F0F0F0F)
        return 1;

    if (lsr(0xFFFFFFFF, 32) != 0)
        return 1;

    return 0;
}

int test_asr(void)
{
    if (asr(0x80F45789, 4) != 0xF80F4578)
        return 1;

    if (asr(0x80000000, 32) != 0xFFFFFFFF)
        return 1;

    return 0;
}

int test_ror(void)
{
    if (ror(0xF0F0F0F0, 4) != 0x0F0F0F0F)
        return 1;

    if (ror(0x12345678, 32) != 0x12345678)
        return 1;

    return 0;
}

int main()
{
    int cnt = 4;
    int outputs[cnt];
    outputs[0] = test_lsl();
    outputs[1] = test_lsr();
    outputs[2] = test_asr();
    outputs[3] = test_ror();

    for (int i = 0; i < cnt; i++) {
        printf("test %d..  ", i);
        if (outputs[i] != 0)
            printf("failed\n");
        else
            printf("ok\n");
    }

    return 0;
}
