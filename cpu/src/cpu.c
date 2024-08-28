#include "execute.h"

bool 
validCondition
(
    uint32_t condition, 
    uint32_t currentProcessStateRegister
)
{
    switch(condition) 
    {
    case EQ:
        return bit(currentProcessStateRegister, Z);
    case NE:
        return !bit(currentProcessStateRegister, Z);
    case CS:
        return bit(currentProcessStateRegister, C);
    case CC:
        return !bit(currentProcessStateRegister, C);
    case MI:
        return bit(currentProcessStateRegister, N);
    case PL:
        return !bit(currentProcessStateRegister, N);
    case VS:
        return bit(currentProcessStateRegister, V);
    case VC:
        return !bit(currentProcessStateRegister, V);
    case HI:
        return bit(currentProcessStateRegister, C) && !bit(currentProcessStateRegister, Z);
    case LS:
        return !bit(currentProcessStateRegister, C) || bit(currentProcessStateRegister, Z);
    case GE:
        return bit(currentProcessStateRegister, N) == bit(currentProcessStateRegister, V);
    case LT:
        return bit(currentProcessStateRegister, N) != bit(currentProcessStateRegister, V);
    case GT:
        return !bit(currentProcessStateRegister, Z) && (bit(currentProcessStateRegister, N) == bit(currentProcessStateRegister, V));
    case LE:
        return bit(currentProcessStateRegister, Z) || (bit(currentProcessStateRegister, N) != bit(currentProcessStateRegister, V));
    case AL:
        return  true;
    default:
        printf("Invalid Condition");
    }

    return false;
}

void 
memoryReference
(
    uint8_t            *pMemory, 
    TemporaryRegisters *pTemporaryRegisters
)
{
    switch(pTemporaryRegisters->operation) {
    case LDR:
        pTemporaryRegisters->loadMemoryData = load32(pMemory, pTemporaryRegisters->ALUOutput);
        break;
    case LDRB:
        pTemporaryRegisters->loadMemoryData = load8(pMemory, pTemporaryRegisters->ALUOutput);
        break;
    case STR:
        store32(pMemory, pTemporaryRegisters->ALUOutput, pTemporaryRegisters->b);
        break;
    case STRB:
        store8(pMemory, pTemporaryRegisters->ALUOutput, pTemporaryRegisters->b);
        break;
    }
} 

void 
registerWriteback
(
    TemporaryRegisters *pTemporaryRegisters, 
    uint32_t registers[]
)
{
    uint32_t rs = bits(pTemporaryRegisters->instruction, 19, 16);
    uint32_t rt = bits(pTemporaryRegisters->instruction, 15, 12);

    if (pTemporaryRegisters->operation == LDR || pTemporaryRegisters->operation == LDRB) 
    {
        registers[rs] = pTemporaryRegisters->singleDataTransferOffset;
        registers[rt] = pTemporaryRegisters->loadMemoryData;
    } 
    else if (pTemporaryRegisters->operation == STR || pTemporaryRegisters->operation == STRB) 
    {
        registers[rs] = pTemporaryRegisters->singleDataTransferOffset;
    } 
    else if (pTemporaryRegisters->operation == DATA && pTemporaryRegisters->writeback) 
    {
        registers[rt] = pTemporaryRegisters->ALUOutput;
    }
    else if (pTemporaryRegisters->operation == MUL)
    {
        registers[rs] = pTemporaryRegisters->ALUOutput;
    }
    else if (pTemporaryRegisters->operation == BRANCH)
    {
        if (pTemporaryRegisters->link)
        {
            registers[LR] = registers[PC];
        }

        registers[PC] += pTemporaryRegisters->ALUOutput;
    }
}

void 
registerFetch
(
    uint32_t instruction, 
    TemporaryRegisters *pTemporaryRegisters,
    uint32_t registers[]
)
{
    uint32_t rs = bits(instruction, 19, 16);
    uint32_t rt = bits(instruction, 15, 12);
    uint32_t ru = bits(instruction, 11, 8);
    uint32_t rv = bits(instruction, 3, 0);

    pTemporaryRegisters->a = registers[rs];
    pTemporaryRegisters->b = registers[rt];
    pTemporaryRegisters->c = registers[ru];
    pTemporaryRegisters->d = registers[rv];
    pTemporaryRegisters->instruction = instruction;
    pTemporaryRegisters->condition = bits(instruction, 31, 28);
}

uint32_t 
decode
(
    uint32_t instruction
)
{
    uint32_t operation;

    if ((instruction & MULT_MASK) == MUL)
    {
        operation = MUL;
    }
    else if ((instruction & SDT_MASK) == LDR)
    {
        operation = LDR;
    }
    else if ((instruction & SDT_MASK) == LDRB)
    {
        operation = LDRB;
    }
    else if ((instruction & SDT_MASK) == STR)
    {
        operation = STR;
    }
    else if ((instruction & SDT_MASK) == STRB)
    {
        operation = STRB;
    }
    else if ((instruction & DATA_MASK) == DATA)
    {
        operation = DATA;
    }
    else if ((instruction & BRANCH_MASK) == BRANCH)
    {
        operation = BRANCH;
    }

    return operation;
}

int 
loadProgram
(
    uint8_t *pMemory, 
    char    *program
)
{
    FILE *f;
    f = fopen(program, "rb");

    if (!f) 
    {
        perror("fopen() failed");
        return -1;
    }

    uint8_t buffer;
    int     i;

    for (i = 0; fread(&buffer, sizeof buffer, 1, f) > 0; i++)
    {
        pMemory[i] = buffer;
    }

    fclose(f);
    return i;
}

int
main
(
    int   argc, 
    char *argv[]
)
{
    if (argc != 2) 
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    uint8_t *pMemory = (uint8_t *)malloc(0x1000);
    int      programSize = loadProgram(pMemory, argv[1]);

    if (programSize == -1) 
    {
        perror("loadProgram() failed");
        return 1;
    }
    
    /* Using last index as CPSR register */
    uint32_t            registers[17] = {0};
    TemporaryRegisters *pTemporaryRegisters;

    pTemporaryRegisters = (TemporaryRegisters *)malloc(sizeof *pTemporaryRegisters);

    while (registers[PC] != programSize) 
    {
        uint32_t instruction = load32(pMemory, registers[PC]);
        registers[PC] += 4;

        registerFetch(instruction, pTemporaryRegisters, registers);
        pTemporaryRegisters->operation = decode(instruction);
        
        if (!validCondition(pTemporaryRegisters->condition, registers[CPSR])) 
        {
            continue;
        }
        
        execute(pTemporaryRegisters, registers);

        memoryReference(pMemory, pTemporaryRegisters);

        registerWriteback(pTemporaryRegisters, registers);
        dump(registers);
    }

    free(pTemporaryRegisters);
    free(pMemory);
    return 0;
}
