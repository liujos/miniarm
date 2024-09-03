#include "execute.h"

void
multiply
(
    TemporaryRegisters *pTemporaryRegisters,
    uint32_t           *pCurrentProcessStateRegister
)
{
    bool     accumulate = bit(pTemporaryRegisters->instruction, 21);
    bool     alterCPSR = bit(pTemporaryRegisters->instruction, 20);
    uint32_t result = pTemporaryRegisters->c * pTemporaryRegisters->d + accumulate * pTemporaryRegisters->b;

    pTemporaryRegisters->ALUOutput = result;
    pTemporaryRegisters->writeback = true;

    if (alterCPSR) 
    {
        *pCurrentProcessStateRegister = changeBit(*pCurrentProcessStateRegister, N, bit(result, 31));
        *pCurrentProcessStateRegister = changeBit(*pCurrentProcessStateRegister, Z, result == 0);
    }
}

void 
barrelShifter
(
    barrelShifterParameters *pShift, 
    uint32_t                 currentProcessStateRegister
)
{
    pShift->carry = 0;

    switch(pShift->type) 
    {
    case LSL:
        pShift->output = logicalShiftLeft(pShift->sequence, pShift->amount);

        if (pShift->amount == 0)
        {
            pShift->carry = bit(currentProcessStateRegister, C);
        }
        else if (pShift->amount <= 32)
        {
            pShift->carry = bit(pShift->sequence, 32 - pShift->amount);
        }
        break;

    case LSR:
        if (pShift->amount == 0)
        {
            pShift->amount = 32;

        }

        pShift->output = logicalShiftRight(pShift->sequence, pShift->amount);

        if (pShift->amount <= 32)
        {
            pShift->carry = bit(pShift->sequence, pShift->amount - 1);
        }
        break;

    case ASR:
        if (pShift->amount == 0 || pShift->amount > 31)
        {
            pShift->amount = 32;
        }

        pShift->output = arithmeticShiftRight(pShift->sequence, pShift->amount);
        pShift->carry = bit(pShift->sequence, pShift->amount - 1);
        break;

    case ROR:
        while (pShift->amount > 32)
        {
            pShift->amount -= 32;
        }

        if (pShift->amount == 0) 
        {
            pShift->output = rotateRightExtended(pShift->sequence, bit(currentProcessStateRegister, C));
            pShift->carry = bit(pShift->sequence, 0);
        } 
        else 
        {
            pShift->output = rotateRight(pShift->sequence, pShift->amount);
            pShift->carry = bit(pShift->sequence, pShift->amount - 1);
        }
        break;
    }
}


barrelShifterParameters
decodeOp2
(
    TemporaryRegisters      *pTemporaryRegisters, 
    uint32_t                 registers[]
)
{
    bool                    immediate = bit(pTemporaryRegisters->instruction, 25);
    bool                    shiftAmountSpecifiedByRegister = bit(pTemporaryRegisters->instruction, 4);
    barrelShifterParameters shift;

    if (immediate) 
    {
        shift.amount = 2 * bits(pTemporaryRegisters->instruction, 11, 8);
        shift.sequence = bits(pTemporaryRegisters->instruction, 7, 0);

        if (shift.amount == 0)
        {
            shift.type = LSL;
        }
    } 
    else if (shiftAmountSpecifiedByRegister) 
    {
        uint32_t shiftRegister = bits(pTemporaryRegisters->instruction, 11, 8);
        shift.amount = bits(registers[shiftRegister], 7, 0);
        shift.sequence = pTemporaryRegisters->d;

        if (shift.amount == 0)
        {
            shift.type = LSL;
        } 
        else 
        {
            shift.type = bits(pTemporaryRegisters->instruction, 6, 5);
        }
    } 
    else 
    {
        shift.amount = bits(pTemporaryRegisters->instruction, 11, 7);
        shift.sequence = pTemporaryRegisters->d;
        shift.type = bits(pTemporaryRegisters->instruction, 6, 5);
    }

    barrelShifter(&shift, registers[CPSR]);
    return shift;
}


void
dataProcessing
(
    TemporaryRegisters *pTemporaryRegisters,
    uint32_t            registers[]
)
{
    barrelShifterParameters shift = decodeOp2(pTemporaryRegisters, registers);

    uint32_t  operand2 = shift.output;
    uint32_t  operand1 = pTemporaryRegisters->a;
    uint32_t  opcode = bits(pTemporaryRegisters->instruction, 24, 21);
    uint32_t  result;
    bool      logicOperation = false;
    bool      writeback = true;
    bool      alterCPSR = bit(pTemporaryRegisters->instruction, 20);

    switch(opcode) {
    case AND:
        result = operand1 & operand2;
        logicOperation = true;
        break;
    case EOR:
        result = operand1 ^ operand2;
        logicOperation = true;
        break;
    case SUB:
        result = operand1 - operand2;
        break;
    case RSB:
        result = operand2 - operand1;
        break;
    case ADD:
        result = operand1 + operand2;
        break;
    case ADC:
        result = operand1 + operand2 + bit(registers[CPSR], C);
        break;
    case SBC:
        result = operand1 - operand2 + bit(registers[CPSR], C) - 1;
        break;
    case RSC:
        result = operand2 - operand1 + bit(registers[CPSR], C) - 1;
        break;
    case TST:
        result = operand1 & operand2;
        logicOperation = true;
        writeback = false;
        break;
    case TEQ:
        result = operand1 ^ operand2;
        logicOperation = true;
        writeback = false;
        break;
    case CMP:
        result = operand1 - operand2;
        logicOperation = true;
        writeback = false;
        break;
    case CMN:
        result = operand1 + operand2;
        writeback = false;
        break;
    case ORR:
        result = operand1 | operand2;
        logicOperation = true;
        break;
    case MOV:
        result = operand2;
        logicOperation = true;
        break;
    case BIC:
        result = operand1 & ~operand2;
        logicOperation = true;
        break;
    case MVN:
        result = ~operand2;
        logicOperation = true;
    }

    pTemporaryRegisters->ALUOutput = result;
    pTemporaryRegisters->writeback = writeback;


    if (alterCPSR)
    {
        registers[CPSR] = changeBit(registers[CPSR], Z, result == 0);
        registers[CPSR] = changeBit(registers[CPSR], N, bit(result, 31));
        
        if (logicOperation)
        {
            registers[CPSR] = changeBit(registers[CPSR], C, shift.carry);
        }
        else
        {
            bool overflow = ((bit(31, operand1) ^ bit(31, operand2)) == 0) && (bit(31, result) != bit(31, operand1));
            bool carry = result < operand1;
            registers[CPSR] = changeBit(registers[CPSR], V, overflow);
            registers[CPSR] = changeBit(registers[CPSR], C, carry);
        }
    }
}

uint32_t 
decodeOffset
(
    TemporaryRegisters *pTemporaryRegisters, 
    uint32_t            currentProcessStateRegister
)
{
    bool immediate = bit(pTemporaryRegisters->instruction, 25);

    if (immediate) 
    {
        return bits(pTemporaryRegisters->instruction, 11, 0);
    } 

    struct barrelShifterParameters shift;
    shift.type = bits(pTemporaryRegisters->instruction, 6, 5);
    shift.amount = bits(pTemporaryRegisters->instruction, 11, 7);
    shift.sequence = pTemporaryRegisters->d;
    barrelShifter(&shift, currentProcessStateRegister);
    return shift.output;
}

void
singleDataTransfer
(
    TemporaryRegisters *pTemporaryRegisters,
    uint32_t            currentProcessStateRegister
)
{
    uint32_t offset = decodeOffset(pTemporaryRegisters, currentProcessStateRegister);
    bool     preindex = bit(pTemporaryRegisters->instruction, 24);
    int      addOffset = bit(pTemporaryRegisters->instruction, 23) ? 1 : -1;
    uint32_t addr = pTemporaryRegisters->a + addOffset * preindex * offset;

    pTemporaryRegisters->singleDataTransferOffset = pTemporaryRegisters->a + addOffset * offset;
    pTemporaryRegisters->ALUOutput = addr;
}

void branch(TemporaryRegisters *pTemporaryRegisters)
{
    uint32_t offset = bits(pTemporaryRegisters->instruction, 23, 0);
    pTemporaryRegisters->link = bit(pTemporaryRegisters->instruction, 24);

    /* assembler assumes the PC is 2 instructions ahead of the current instruction
       but we add only 4 because our PC is 1 instruction ahead since it's not pipelined */

    pTemporaryRegisters->ALUOutput = arithmeticShiftRight(offset << 8, 6) + 4;
}

void 
execute
(
    TemporaryRegisters *pTemporaryRegisters, 
    uint32_t registers[]
)
{
    switch(pTemporaryRegisters->operation) {
    case MUL:
        multiply(pTemporaryRegisters, &registers[CPSR]);
        break;
    case DATA:
        dataProcessing(pTemporaryRegisters, registers);
        break;
    case LDR:
    case LDRB:
    case STR:
    case STRB:
        singleDataTransfer(pTemporaryRegisters, registers[CPSR]);
        break;
    case BRANCH:
        branch(pTemporaryRegisters);
        break;
    }
}
