#include <stdint.h>
#include <stdio.h>
#include "cpu.h"

typedef uint8_t u8;
typedef uint32_t u32;

u32 bit(u32 sequence, u32 i);
u32 bits(u32 sequence, u32 start, u32 end);
u32 slr(u32 sequence, u32 shift_amount);
u32 sar(u32 sequence, u32 shift_amount);
u32 ror(u32 sequence, u32 shift_amount);
u32 rorx(u32 sequence, u32 *carry);
void dump(u32 *register_file);
