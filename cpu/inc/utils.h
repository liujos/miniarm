#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

uint32_t logicalShiftLeft(uint32_t sequence, uint32_t amount);
uint32_t logicalShiftRight(uint32_t sequence, uint32_t amount);
uint32_t arithmeticShiftRight(uint32_t sequence, uint32_t amount);
uint32_t rotateRight(uint32_t sequence, uint32_t amount);
uint32_t rotateRightExtended(uint32_t sequence, uint32_t carry);
uint32_t bit(uint32_t sequence, uint32_t index);
uint32_t bits(uint32_t sequence, uint32_t start, uint32_t end);
uint32_t changeBit(uint32_t sequence, uint32_t index, bool status);
void     dump(uint32_t registers[]);

#endif
