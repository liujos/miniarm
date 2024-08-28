#!/usr/bin/python3
from abc import ABC, abstractmethod
import re

def parse_register(reg: str) -> int:
    try:
        x = int(reg[1:])
    except ValueError:
        raise SyntaxError

    if x < 0 or x > 15:
        raise SyntaxError
    return x

def parse_immediate(imm: str) -> int:
    try:
        x = int(imm[1:])
    except ValueError:
        raise SyntaxError

    return x

def is_imm(imm: str) -> bool:
    return imm[0] == '#'

def rol(sequence: int, amount: int) -> int:
    return ((sequence << amount) | (sequence >> (32 - amount))) & 0xFFFFFFFF



class Instruction(ABC):
    CONDS = {'EQ': 0b0000, 'NE': 0b0001, 'CS': 0b0010, 'CC': 0b0011, 'MI': 0b0100,
             'PL': 0b0101, 'VS': 0b0110, 'VC': 0b0111, 'HI': 0b1000, 'LS': 0b1001,
             'GE': 0b1010, 'LT': 0b1011, 'GT': 0b1100, 'LE': 0b1101, 'AL': 0b1110}

    COND_RE = '(EQ|NE|CS|CC|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE)?'

    def __init__(self, line: str):
        self.line = line
        self.encoding = 0
        self.cond = 0

    @abstractmethod
    def tokenize(self):
        pass

    @abstractmethod
    def parse_line(self):
        pass

    @abstractmethod
    def encode(self):
        pass



class Branch(Instruction):
    MATCH_RE = '(B)(L)?' + Instruction.COND_RE + '$'

    def __init__(self, line: list[str], symbol_table: dict[str, int], location: int):
        super().__init__(line)
        self.symbol_table = symbol_table
        self.location = location

    def tokenize(self):
        self.tokens = self.line.split()

    #b{l}{cond} <label>
    def parse_line(self):
        self.tokenize()
        m = re.match(self.MATCH_RE, self.tokens[0]) 

        if not m:
            raise SyntaxError

        self.link = bool(m.group(2)) 
        self.cond = self.CONDS[m.group(3)] if m.group(3) else self.CONDS['AL']
        self.label = self.tokens[1]

    def encode(self):
        self.encoding |= self.cond << 28
        self.encoding |= 0b101 << 25
        self.encoding |= self.link << 24

        #subtract 8 because PC is 2 instructions ahead due to pipelining
        self.offset = (((self.symbol_table[self.label] - self.location) * 4 - 8) >> 2) & 0xFFFFFF
        self.encoding |= self.offset



class DataProcessing(Instruction):
    OPCODES = {'AND': 0b0000, 'EOR': 0b0001, 'SUB': 0b0010, 'RSB': 0b0011, 
               'ADD': 0b0100, 'ADC': 0b0101, 'SBC': 0b0110, 'RSC': 0b0111, 
               'TST': 0b1000, 'TEQ': 0b1001, 'CMP': 0b1010, 'CMN': 0b1011, 
               'ORR': 0b1100, 'MOV': 0b1101, 'BIC': 0b1110, 'MVN': 0b1111}

    MATCH_RE = '(AND|EOR|SUB|RSB|ADD|ADC|SBC|RSC|TST|TEQ|CMP|CMN|ORR|MOV|BIC|MVN)'\
                                                   + Instruction.COND_RE + '(S)?$'

    SHIFT_CODE = {'LSL': 0b00, 'LSR': 0b01, 'ASR': 0b10, 'ROR': 0b11}

    def __init__(self, line: list[str]):
        super().__init__(line)

    def tokenize(self):
        self.tokens = self.line.replace(',', ' ')
        self.tokens = self.tokens.split()

    @staticmethod
    def valid_shift(type: int, amount: int) -> bool:
        if type == DataProcessing.SHIFT_CODE['LSL'] and amount == 32:
            return False
        return amount >= 0 and amount <= 32

    @staticmethod
    def encode_shift(shift: list[str]) -> int:
        #rrx is encoded as ror #0
        if shift[0] == 'RRX':
            return DataProcessing.SHIFT_CODE['ROR'] << 1

        shift_type = DataProcessing.SHIFT_CODE[shift[0]]
        if shift[1][0] == 'R':
            shift_reg = parse_register(shift[1])
            return 1 | (shift_type) << 1 | (shift_reg << 4)

        shift_amount = parse_immediate(shift[1])
        if not DataProcessing.valid_shift(shift_type, shift_amount):
            raise ValueError

        #lsr #0, asr #0, ror #0 are encoded as lsl #0
        if shift_amount == 0:
            shift_type = DataProcessing.SHIFT_CODE['LSL']

        #lsr #32, asr #32 are encoded as lsr #0, asr #0
        if shift_amount == 32:
            shift_amount = 0

        return (shift_type << 1) | (shift_amount << 3) 

    @staticmethod
    def compress(imm: int) -> int:
        for rot in range(16):
            if imm < 256 and imm >= 0:
                return imm, rot
            imm = rol(imm, 2)
        raise ValueError
        
    def encode_op2(self, op2: list[str]) -> int:
        if not op2:
            raise SyntaxError(f"Missing operand: {self.line}")

        if is_imm(op2[0]):
            self.is_imm = True
            imm = parse_immediate(op2[0])
            imm, rot = self.compress(imm)
            return (rot << 8) | imm

        rm = parse_register(op2[0])
        if len(op2) == 1:
            return rm

        shift = self.encode_shift(op2[1:])
        return (shift << 4) | rm

    def parse_line(self):
        self.tokenize()
        m = re.match(self.MATCH_RE, self.tokens[0])

        if not m:
            raise SyntaxError

        op = m.group(1)
        self.cond = self.CONDS[m.group(2)] if m.group(2) else self.CONDS['AL']
        self.opcode = self.OPCODES[op]
        self.is_imm = False
        self.alter_cpsr = True if m.group(3) else False

        if op in {'CMP', 'CMN', 'TEQ', 'TST'}:
            self.alter_cpsr = True
            self.rn = parse_register(self.tokens[1])
            self.rd = 0
            self.op2 = self.encode_op2(self.tokens[2:])
        elif op in {'MOV', 'MVN'}:
            self.rn = 0
            self.rd = parse_register(self.tokens[1])
            self.op2 = self.encode_op2(self.tokens[2:])
        else:
            self.rd = parse_register(self.tokens[1])
            self.rn = parse_register(self.tokens[2])
            self.op2 = self.encode_op2(self.tokens[3:])

    def encode(self):
        self.encoding |= self.cond << 28
        self.encoding |= self.is_imm << 25
        self.encoding |= self.opcode << 21
        self.encoding |= self.alter_cpsr << 20
        self.encoding |= self.rn << 16
        self.encoding |= self.rd << 12
        self.encoding |= self.op2



class Multiply(Instruction):
    MATCH_RE = '(MUL|MLA)' + Instruction.COND_RE + '(S)?$'

    def __init__(self, line: list[str]):
        super().__init__(line)

    def tokenize(self):
        self.tokens = self.line.replace(',', ' ')
        self.tokens = self.tokens.split()

    def parse_line(self):
        self.tokenize()
        m = re.match(self.MATCH_RE, self.tokens[0])

        if not m:
            raise SyntaxError

        op = m.group(1)
        self.cond = self.CONDS[m.group(2)] if m.group(2) else self.CONDS['AL']
        self.accumulate = op == 'MLA'
        self.alter_cpsr = True if m.group(3) else False
        self.rd = parse_register(self.tokens[1])
        self.rm = parse_register(self.tokens[2])
        self.rs = parse_register(self.tokens[3])
        self.rn = parse_register(self.tokens[4]) if self.accumulate else 0

    def encode(self):
        self.encoding |= self.cond << 28
        self.encoding |= self.accumulate << 21
        self.encoding |= self.alter_cpsr << 20
        self.encoding |= self.rd << 16
        self.encoding |= self.rn << 12
        self.encoding |= self.rs << 8
        self.encoding |= 0b1001 << 4
        self.encoding |= self.rm



class SingleDataTransfer(Instruction):
    MATCH_RE = '(LDR|STR)' + Instruction.COND_RE + '(B)?(T)?'

    def __init__(self, line: list[str]):
        super().__init__(line)

    def tokenize(self):
        self.tokens = self.line.replace(',', ' ')

        addr = self.tokens.split(maxsplit=2)[2]
        self.tokens = self.tokens.split(maxsplit=2)[0:2]
        m = re.match('\[(.*)\]', addr)
        inside_delimiter = m.group(1).strip()

        addr = addr.replace('[', ' ', 1)
        addr = addr.replace(']', ' ', 1)
        addr = addr.split()
        is_preindex = len(inside_delimiter) > 2 or len(addr) == 1
        self.tokens.append(is_preindex)

        sign = '+'
        if len(addr) > 1:
            if addr[1][0] == '#' and addr[1][1] == '-': 
                sign = '-'
                addr[1] = addr[1][0] + addr[1][2:]
            elif addr[1][0] == '#' and addr[1][1] == '+':
                addr[1] = addr[1][0] + addr[1][2:]
            elif addr[1][1] == 'R' and addr[1][0] == '-':
                sign = '-'
                addr[1] = addr[1][1:]
            elif addr[1][1] == 'R' and addr[1][0] == '+':
                addr[1] = addr[1][1:]

        self.tokens.append(sign)
        self.tokens += addr

    @staticmethod
    def encode_shift(shift: list[str]) -> int:
        #rrx is encoded as ror #0
        if shift[0] == 'RRX':
            return DataProcessing.SHIFT_CODE['ROR'] << 1

        shift_type = DataProcessing.SHIFT_CODE[shift[0]]
        shift_amount = parse_immediate(shift[1])
        if not DataProcessing.valid_shift(shift_type, shift_amount):
            raise ValueError

        #lsr #0, asr #0, ror #0 are encoded as lsl #0
        if shift_amount == 0:
            shift_type = DataProcessing.SHIFT_CODE['LSL']

        #lsr #32, asr #32 are encoded as lsr #0, asr #0
        if shift_amount == 32:
            shift_amount = 0

        return (shift_type << 1) | (shift_amount << 3) 

    def encode_offset(self, offset) -> int:
        if offset[0][0] == '#':
            imm = parse_immediate(offset[0])
            return imm

        self.is_register_specified = True
        rm = parse_register(offset[0])
        if len(offset) == 1:
            return rm

        shift = self.encode_shift(offset[1:])
        return rm | (shift << 4)

    def parse_addr(self, addr):
        self.rn = parse_register(addr[0])

        if len(addr) == 1:
            self.offset = 0
            return

        self.offset = self.encode_offset(addr[1:])

    def parse_line(self):
        self.tokenize()
        m = re.match(self.MATCH_RE, self.tokens[0])

        if not m:
            raise SyntaxError

        op = m.group(1)
        self.is_register_specified = False
        self.is_preindex = self.tokens[2]
        self.is_writeback = False
        self.is_load = op == 'LDR' 
        self.cond = self.CONDS[m.group(2)] if m.group(2) else self.CONDS['AL']
        self.is_byte = True if m.group(3) else False
        self.rd = parse_register(self.tokens[1])
        self.is_up = self.tokens[3] == '+'
        self.parse_addr(self.tokens[4:])

        if m.group(4) and not self.is_preindex:
            self.is_writeback = True
        elif m.group(4) and self.is_preindex:
            raise SyntaxError
        elif self.tokens[-1] == '!' and self.is_preindex:
            self.is_writeback = True


    def encode(self):
        self.encoding |= self.cond << 28
        self.encoding |= 0b01 << 26
        self.encoding |= self.is_register_specified << 25
        self.encoding |= self.is_preindex << 24
        self.encoding |= self.is_up << 23
        self.encoding |= self.is_byte << 22
        self.encoding |= self.is_writeback << 21
        self.encoding |= self.is_load << 20
        self.encoding |= self.rn << 16
        self.encoding |= self.rd << 12
        self.encoding |= self.offset
