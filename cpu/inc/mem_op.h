#ifndef MEM_OP_H
#define MEM_OP_H

#include "utils.h"

uint32_t load8(uint8_t *pMemory, uint8_t address);
uint32_t load32(uint8_t *pMemory, uint32_t address);
void store8(uint8_t *pMemory, uint32_t address, uint8_t data);
void store32(uint8_t *pMemory, uint32_t address, uint32_t data);

#endif
