#include <assert.h>
#include "cpu.h"

enum Ops {
    DATA,
    MUL,
    LOAD,
    STORE
};

struct instruction_segments {
    u32 cond;
    u32 funct;
    u32 rwi;
    u32 rxi;
    u32 ryi;
    u32 rzi;
    u32 offset;
};

struct arm7 {
    struct instruction_segments ins;
    u32 regs[REGISTERS];
    u8 cpsr[4];
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

u32 get_offset(struct arm7 *cpu)
{
    bool immediate = bit(cpu->ins.funct, 5);

    if (immediate) 
        return cpu->ins.offset;

    u32 rm = cpu->regs[cpu->ins.rzi];
    u32 shift_amount = bits(cpu->ins.offset, 11, 4);
    u32 offset = 0;

    switch(shift_type) {
    case LSL:
        offset = rm << shift_amount;
        break;
    case LSR:
        if (shift_amount != 0)
            offset = slr(rm, shift_amount);
        break;
    case ASR:
        if (shift_amount == 0)
            offset = bit(rm, 31) == 1 ? 0xFFFFFFFF : 0;
        else
            offset = sar(rm, shift_amount);
        break;
    case ROR:
        if (shift_amount == 0) 
            offset = rorx(rm, &cpu->cpsr[C]);
        else 
            offset = ror(rm, shift_amount);
        break;
    }

    return offset;
}

u32 single_data_transfer(struct arm7 *cpu)
{
    bool modify_before = bit(cpu->ins.funct, 4);
    bool add_offset = bit(cpu->ins.funct, 3);
    bool transfer_byte = bit(cpu->ins.funct, 2);
    bool writeback = bit(cpu->ins.funct, 1);

    u32 rn = cpu->regs[cpu->ins.rwi];
    u32 offset = get_offset();
    
    u32 addr = rn;

    if (add_offset) 
        rn += offset;
    else 
        rn -= offset;
    
    if (modify_before) {
        addr = rn;

    if (writeback) 
        cpu->regs[cpu->ins.rwi] = rn;
    
    return addr;
}

void multiply(struct arm7 *cpu)
{
    /* TODO: destination register must not be the same as rm 
     * r15 must not be used as an operand or destination register */

    bool accumulate = bit(cpu->ins.funct, 21);
    bool set_cpsr = bit(cpu->ins.funct, 20);

    u32 rn = cpu->regs[cpu->ins.rxi];
    u32 rs = cpu->regs[cpu->ins.ryi];
    u32 rm = cpu->regs[cpu->ins.rzi];
    u32 *rd = &cpu->regs[cpu->ins.rwi];
    u32 res = rm * rs;

    if (accumulate)
        res += rn;

    if (set_cpsr) {
        cpu->cpsr[N] = bit(res, 31);
        cpu->cpsr[Z] = res == 0 ? 1 : 0;
    }

    *rd = res;
}

u32 decode(u32 instruction, struct instruction_segments *ins)
{
    ins->cond = bits(instruction, 31, 28);
    ins->funct = bits(instruction, 25, 20);
    ins->rwi = bits(instruction, 19, 16);
    ins->rxi = bits(instruction, 15, 12);
    ins->ryi = bits(instruction, 11, 8);
    ins->rzi = bits(instruction, 3, 0);
    ins->offset = bits(instruction, 11, 0);

    if (bits(instruction, 27, 26) == 0)
        return DATA;
    else if (bits(instruction, 27, 22) == 0 && bits(instruction, 7, 4) == 0b1001)
        return MUL;
}

u32 get_operand2(struct arm7 *cpu, u32 *carry_out)
{
    u32 rm = cpu->regs[cpu->ins.rzi];
    u32 op2 = 0;
    u32 shift_type;
    u32 shift_amount;

    bool immediate = bit(cpu->ins.funct, 5);
    bool register_shift = bit(cpu->ins.offset, 4);

    if (immediate) {
        *carry_out = bit(rm, shift_amount - 1);
        op2 = ror(bits(cpu->ins.offset, 7, 0), 2 * bits(cpu->ins.offset, 11, 8));
        return op2;
    }

    shift_type = bits(cpu->ins.offset, 6, 5);
    if (register_shift) {
        u32 rsi = bits(cpu->ins.offset, 11, 8);
        shift_amount = bits(cpu->regs[rsi], 7, 0);
        if (shift_amount == 0)
            shift_type = LSL;
    } else {
        shift_amount = bits(cpu->ins.offset, 11, 7);
    }

    switch(shift_type) {
    case LSL:
        if (shift_amount == 0) {
            op2 = rm;
        } else if (shift_amount < 32) {
            *carry_out = bit(rm, 32 - shift_amount);
            op2 = rm << shift_amount;
        } else if (shift_amount == 32) {
            *carry_out = bit(rm, 0);
        } else {
            *carry_out = 0;
        }
        break;
    case LSR:
        if (shift_amount > 0 && shift_amount < 32) {
            *carry_out = bit(rm, shift_amount - 1);
            op2 = slr(rm, shift_amount);
        } else if (shift_amount > 32) {
            *carry_out = 0;
        } else {
            *carry_out = bit(rm, 31);
        }
        break;
    case ASR:
        if (shift_amount > 0 && shift_amount < 32) {
            *carry_out = bit(rm, shift_amount - 1);
            op2 = sar(rm, shift_amount);
        } else {
            *carry_out = bit(rm, 31);
            op2 = *carry_out == 1 ? 0xFFFFFFFF : 0;
        }
        break;
    case ROR:
        while (shift_amount > 32)
            shift_amount -= 32;

        if (shift_amount == 0) {
            rorx(rm, carry_out);
        } else if (shift_amount < 32) {
            *carry_out = bit(rm, shift_amount - 1);
            op2 = ror(rm, shift_amount);
        } else {
            *carry_out = bit(rm, 31);
            op2 = rm;
        }
        break;
    }
    return op2;
}

void data_processing(struct arm7 *cpu)
{
    u32 carry_out = cpu->cpsr[C];
    u32 op1 = cpu->regs[cpu->ins.rwi];
    u32 op2 = get_operand2(cpu, &carry_out);
    u32 res;
    u32 opcode = bits(cpu->ins.funct, 4, 1);
    u32 *rd = &cpu->regs[cpu->ins.rxi];

    bool set_cpsr = bit(cpu->ins.funct, 0);
    bool logic_op = false;
    bool register_writeback = false;

    switch(opcode) {
    case AND:
        res = op1 & op2;
        register_writeback = true;
        logic_op = true;
        break;
    case EOR:
        res = op1 ^ op2;
        register_writeback = true;
        logic_op = true;
        break;
    case SUB:
        res = op1 - op2;
        register_writeback = true;
        break;
    case RSB:
        res = op2 - op1;
        register_writeback = true;
        break;
    case ADD:
        res = op1 + op2;
        register_writeback = true;
        break;
    case ADC:
        res = op1 + op2 + cpu->cpsr[C];
        register_writeback = true;
        break;
    case SBC:
        res = op1 - op2 + cpu->cpsr[C] - 1;
        register_writeback = true;
        break;
    case RSC:
        res = op2 - op1 + cpu->cpsr[C] - 1;
        register_writeback = true;
        break;
    case TST:
        res = op1 & op2;
        logic_op = true;
        break;
    case TEQ:
        res = op1 ^ op2;
        logic_op = true;
        break;
    case CMP:
        res = op1 - op2;
        break;
    case CMN:
        res = op1 + op2;
        break;
    case ORR:
        res = op1 | op2;
        register_writeback = true;
        logic_op = true;
        break;
    case MOV:
        res = op2;
        register_writeback = true;
        logic_op = true;
        break;
    case BIC:
        res = op1 & ~op2;
        register_writeback = true;
        logic_op = true;
        break;
    case MVN:
        res = ~op2;
        register_writeback = true;
        logic_op = true;
    default:
        printf("Undefined Instruction\n");
    }

    /*if (s_bit) {
        if (logic_op) {
            cpu->cpsr[C] = c_flag;
        } else {
            cpu->cpsr[V] = ;
            cpu->cpsr[C]
        }
        cpu->cpsr[Z] = res == 0 ? 1 : 0;
        cpu->cpsr[N] = bit(res, 31);
    }*/

    if (register_writeback)
        *rd = res;
}

int main()
{
    struct arm7 cpu;
    for (int i = 0; i < REGISTERS; i++)
        cpu.regs[i] = 0;

    cpu.regs[1] = 0b1110;
    cpu.regs[3] = 0b1010;
    dump(cpu.regs);

    /* Instruction Fetch */
    u32 instruction = 0xF0012013; //rf[2] = rf[1] & (rf[3] << 1);

    /* Instruction Decode */
    decode(instruction, &cpu.ins);

    /* Execute */
    data_processing(&cpu);
    dump(cpu.regs);
    assert(cpu.regs[2] == (cpu.regs[1] & (cpu.regs[3] << 1)));

    return 0;
} 

