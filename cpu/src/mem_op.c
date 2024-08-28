#include "mem_op.h"

uint32_t 
load8
(
    uint8_t *pMemory, 
    uint8_t  address
)
{
    return pMemory[address];
}

uint32_t 
load32
(
    uint8_t *pMemory, 
    uint32_t address
)
{
    uint32_t wordBoundaryOffset = address % 4;
    address -= wordBoundaryOffset;
    uint32_t data = pMemory[address] | pMemory[address + 1] << 8 | pMemory[address + 2] << 16 | pMemory[address + 3] << 24;
    return rotateRight(data, 8 * wordBoundaryOffset);
}

void 
store8
(
    uint8_t *pMemory, 
    uint32_t address, 
    uint8_t  data
)
{
    pMemory[address] = data;
}

void 
store32
(
    uint8_t *pMemory, 
    uint32_t address, 
    uint32_t data
)
{
    pMemory[address] = bits(data, 7, 0);
    pMemory[address + 1] = bits(data, 15, 8);
    pMemory[address + 2] = bits(data, 23, 16);
    pMemory[address + 3] = bits(data, 31, 24);
} 
