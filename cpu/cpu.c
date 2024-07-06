#include <stdlib.h>
#include "utils.h"
#include "cpu.h"

#define PC 15

u8 mem[0x1000];

enum Ops {
    DATA,
    MUL,
    SDT,
    BRANCH
};

struct shift
{
    u32 carry;
    u32 output;
    u32 sequence;
    u32 amount;
    enum ShiftType type;
};

struct id_ex {
    u32 a;
    u32 b;
    u32 c;
    u32 d;
    u32 ir;
    enum Ops op;
};

struct ex_mem {
    u32 ir;
    u32 alu_output;
    u32 b;
    u32 offset;
    enum Ops op;
};

struct mem_wb {
    u32 ir;
    u32 alu_output;
    u32 lmd;
};

bool is_valid_condition(u32 cond, u8 *cpsr)
{
    switch(cond) {
    case EQ:
        return cpsr[Z];
    case NE:
       return !cpsr[Z];
    case CS:
        return cpsr[C];
    case CC:
        return !cpsr[C];
    case MI:
        return cpsr[N];
    case PL:
        return !cpsr[N];
    case VS:
        return cpsr[V];
    case VC:
        return !cpsr[V];
    case HI:
        return cpsr[C] && !cpsr[Z];
    case LS:
        return !cpsr[C] || cpsr[Z];
    case GE:
        return cpsr[N] == cpsr[V];
    case LT:
        return cpsr[N] != cpsr[V];
    case GT:
        return !cpsr[Z] && (cpsr[N] == cpsr[V]);
    case LE:
        return cpsr[Z] || (cpsr[N] != cpsr[V]);
    case AL:
        return true;
    }
    return false;
}

u32 set_bit(u32 sequence, u32 index, bool status)
{
    return (sequence & ~(1 << index)) | (status << index);
}


u32 multiply(struct id_ex id_ex, u32 *regs)
{
    bool is_accumulate = bit(id_ex.ir, 21);
    bool is_condition = bit(id_ex.ir, 20);
    u32 res = id_ex.c * id_ex.d;

    if (is_accumulate)
        res += id_ex.b;

    if (is_condition) {
        regs[CPSR] = set_bit(regs[CPSR], N, bit(res, 31));
        regs[CPSR] = set_bit(regs[CPSR], Z, res == 0);
    }
        
    return res;
}

void barrel_shifter(struct shift *shift, u32 *regs)
{
    shift->carry = 0;

    switch(shift->type) {
    case LSL:
        shift->output = lsl(shift->sequence, shift->amount);

        if (shift->amount == 0)
            shift->carry = bit(regs[CPSR], C);
        else if (shift->amount <= 32)
            shift->carry = bit(shift->sequence, 32 - shift->amount);
        break;

    case LSR:
        if (shift->amount == 0)
            shift->amount = 32;

        shift->output = lsr(shift->sequence, shift->amount);

        if (shift->amount <= 32)
            shift->carry = bit(shift->sequence, shift->amount - 1);
        break;

    case ASR:
        if (shift->amount == 0 || shift->amount > 31)
            shift->amount = 32;

        shift->output = asr(shift->sequence, shift->amount);
        shift->carry = bit(shift->sequence, shift->amount - 1);
        break;

    case ROR:
        while (shift->amount > 32)
            shift->amount -= 32;

        if (shift->amount == 0 && !shift->is_imm) {
            shift->output = rrx(shift->sequence, bit(regs[CPSR], C));
            shift->carry = bit(shift->sequence, 0);
        } else {
            shift->output = ror(shift->sequence, shift->amount);
            shift->carry = bit(shift->sequence, shift->amount - 1);
        }
    }
}


struct shift decode_op2(struct id_ex id_ex, u32 *regs)
{
    struct shift shift;
    bool is_imm = bit(id_ex.ir, 25);
    bool is_register_specified = bit(id_ex.ir, 4);

    if (is_imm) {
        shift.amount = 2 * bits(id_ex.ir, 11, 8);
        shift.sequence = bits(id_ex.ir, 7, 0);
        shift.type = (shift.amount == 0) ? LSL : ROR;
    } else if (is_register_specified) {
        u32 rs = bits(id_ex.ir, 11, 8);
        shift.amount = bits(regs[rs], 7, 0);
        shift.sequence = id_ex.d;
        shift.type = (shift.amount == 0) ? LSL : bits(id_ex.ir, 6, 5);
    } else {
        shift.amount = bits(id_ex.ir, 11, 7);
        shift.sequence = id_ex.d;
        shift.type = bits(id_ex.ir, 6, 5);
    }

    barrel_shifter(&shift, regs);
    return shift;
}


u32 data_process(struct id_ex id_ex, u32 *regs)
{
    struct shift shift = decode_op2(id_ex, regs);
    u32 op2 = shift.output;
    u32 op1 = id_ex.a;
    u32 res;
    u32 opcode = bits(id_ex.ir, 24, 21);
    bool is_logic_op = false;

    switch(opcode) {
    case AND:
        res = op1 & op2;
        is_logic_op = true;
        break;
    case EOR:
        res = op1 ^ op2;
        is_logic_op = true;
        break;
    case SUB:
        res = op1 - op2;
        break;
    case RSB:
        res = op2 - op1;
        break;
    case ADD:
        res = op1 + op2;
        break;
    case ADC:
        res = op1 + op2 + bit(regs[CPSR], C);
        break;
    case SBC:
        res = op1 - op2 + bit(regs[CPSR], C) - 1;
        break;
    case RSC:
        res = op2 - op1 + bit(regs[CPSR], C) - 1;
        break;
    case TST:
        res = op1 & op2;
        is_logic_op = true;
        break;
    case TEQ:
        res = op1 ^ op2;
        is_logic_op = true;
        break;
    case CMP:
        res = op1 - op2;
        break;
    case CMN:
        res = op1 + op2;
        break;
    case ORR:
        res = op1 | op2;
        is_logic_op = true;
        break;
    case MOV:
        res = op2;
        is_logic_op = true;
        break;
    case BIC:
        res = op1 & ~op2;
        is_logic_op = true;
        break;
    case MVN:
        res = ~op2;
        is_logic_op = true;
    }

    return res;
}

u32 decode_offset(struct id_ex id_ex, u32 *regs)
{
    bool is_imm = bit(id_ex.ir, 25);
    if (is_imm) 
        return bits(id_ex.ir, 11, 0);

    struct shift shift;
    shift.type = bits(id_ex.ir, 6, 5);
    shift.amount = bits(id_ex.ir, 11, 7);
    shift.sequence = id_ex.d;
    barrel_shifter(&shift, regs);
    return shift.output;
}

u32 single_data_transfer(struct id_ex id_ex, u32 *regs, struct ex_mem *ex_mem)
{
    u32 offset = decode_offset(id_ex, regs);
    bool is_preindex = bit(id_ex, 24);
    bool is_up = bit(id_ex, 23);

    if (is_up) {
        ex_mem->offset = id_ex.a + offset;
        return id_ex.a + is_preindex * offset;
    } 

    ex_mem->offset = id_ex.a - offset;
    return id_ex.a - is_preindex * offset;
    
}

void register_writeback(struct mem_wb mem_wb, u32 *regs)
{
    if (mem_wb.op == SDT && bit(mem_wb.ir, 20))
        regs[bits(mem_wb.ir, )]
}

void memory_reference(struct ex_mem ex_mem, struct mem_wb *mem_wb)
{
    mem_wb->ir = ex_mem.ir;
    mem_wb->alu_output = ex_mem.alu_output;

    if (ex_mem.op != SDT)
        return;

    bool is_load = bit(ex_mem.ir, 20);
    if (is_load)
        mem_wb->lmd = mem[ex_mem.alu_output];
    else
        mem[ex_mem.alu_output] = ex_mem.b;
} 

void execute(struct id_ex id_ex, struct ex_mem *ex_mem, u32 *regs)
{
    u32 res;

    switch(id_ex.op) {
    case MUL:
        res = multiply(id_ex, regs);
        break;
    case DATA:
        res = data_process(id_ex, regs);
        break;
    case SDT:
        res = single_data_transfer(id_ex, regs, ex_mem);
        break;
    case BRANCH:
        break;
    }

    //printf("%d\n", res);
    ex_mem->ir = id_ex.ir;
    ex_mem->alu_output = res;
    ex_mem->op = id_ex.op;
}

void register_fetch(u32 instruction, struct id_ex *id_ex, u32 *regs)
{
    id_ex->a = regs[bits(instruction, 19, 16)];
    id_ex->b = regs[bits(instruction, 15, 12)];
    id_ex->c = regs[bits(instruction, 11, 8)];
    id_ex->d = regs[bits(instruction, 3, 0)];
    id_ex->ir = instruction;
}

void decode(u32 instruction, struct id_ex *id_ex)
{
    /* Multiply */
    if (bits(instruction, 27, 22) == 0 && bits(instruction, 7, 4) == 0b1001)
        id_ex->op = MUL;

    /* Single Data Transfer */
    else if (bits(instruction, 27, 26) == 0b01)
        id_ex->op = SDT;

    /* Branch */
    else if (bits(instruction, 27, 25) == 0b101)
        id_ex->op = BRANCH;

    /* Data Processing */
    else if (bits(instruction, 27, 26) == 0)
        id_ex->op = DATA;
}

u32 fetch(u32 addr)
{
    return mem[addr] | mem[addr + 1] << 8 | mem[addr + 2] << 16 | mem[addr + 3] << 24;
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    /* Loads program into memory */
    FILE *f;
    f = fopen(argv[1], "rb");

    if (f == NULL) {
        perror("Error");
        return 1;
    }

    int i;
    u8 buf;
    for (i = 0; fread(&buf, sizeof buf, 1, f) > 0; i++)
        mem[i] = buf;
    fclose(f);

    /* Using last index as CPSR register */
    u32 regs[17] = {0};
    int cnt = i / 4;
    while (cnt--) {
        /* Fetch instruction from memory */
        u32 instruction = fetch(regs[PC]);
        regs[PC] += 4;
        //printf("%x\n", instruction);

        /* Decode instruction / register fetch */
        struct id_ex id_ex;
        register_fetch(instruction, &id_ex, regs);
        decode(instruction, &id_ex);

        /* Execute instruction */
        struct ex_mem ex_mem;
        execute(id_ex, &ex_mem, regs);

        /* Memory reference */
        struct mem_wb mem_wb;
        memory_reference(ex_mem, &mem_wb);

        /* Writeback */
        register_writeback(mem_wb);
    }

    return 0;
}
