import re
from armasm.instructions import *
import struct

#first pass: 
#   omit all leading white spaces
#   omit all comments (all characters following a ';')
#   create a symbol table (label : location)

#second pass: each line is simply an operation
#   translate each mneumonic to its corresponding opcode
#   last two letters of instruction pneumonic will correspond to a condition code
#   grab operands

class AssemblyParser:
    def __init__(self):
        self.location = 0
        self.symbol_table = {}
        self.instructions = []

    def assemble(self, input: str, output = None):
        with open(input, "r") as file:
            self.program = file.read().upper().splitlines()

        self.first_pass()
        self.second_pass(output)

    def first_pass(self):
        for i, line in enumerate(self.program):
            line = line.split(';', 1)[0]
            line = line.strip()

            if m := re.match('(.*)(:)', line):
                self.symbol_table[m.group(1)] = self.location
                line = line[len(m[0]):].lstrip()

            if not line:
                continue

            op = line.split(' ', 1)[0]

            if op[0] == 'B':
                self.instructions.append(Branch(line, self.symbol_table, self.location))
            elif op[0:3] in DataProcessing.OPCODES:
                self.instructions.append(DataProcessing(line))
            elif op[0:3] == 'MUL' or op[0:3] == 'MLA':
                self.instructions.append(Multiply(line))
            elif op[0:3] == 'LDR' or op[0:3] == 'STR':
                self.instructions.append(SingleDataTransfer(line))
            else:
                raise SyntaxError(line)


            self.instructions[-1].parse_line()
            self.location += 1
            self.program[i] = line

    def second_pass(self, output):
        output = 'program.bin' if not output else output
        with open(output, 'wb') as f:
            for ins in self.instructions:
                ins.encode()
                f.write(struct.pack('<I', ins.encoding))


