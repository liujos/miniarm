#!/usr/bin/python3
import unittest
from armasm.instructions import Multiply

class TestMultiply(unittest.TestCase):
    def test1(self):
        i1 = Multiply('MULNES R1,R2,R3')
        i1.parse_line()
        i1.encode()
        self.assertEqual(i1.encoding, 0x10110392)

    def test2(self):
        i2 = Multiply('MULS R4, R5, R6')
        i2.parse_line()
        i2.encode()
        self.assertEqual(i2.encoding, 0xE0140695)

    def test3(self):
        i3 = Multiply('MUL  R7,   R8, R9')
        i3.parse_line()
        i3.encode()
        self.assertEqual(i3.encoding, 0xE0070998)

    def test4(self):
        i4 = Multiply('MLAGTS  R10,   R11, R12, R13')
        i4.parse_line()
        i4.encode()
        self.assertEqual(i4.encoding, 0xC03ADC9B)

    def test5(self):
        i5 = Multiply('MLAS R1, R5,  R10, R11')
        i5.parse_line()
        i5.encode()
        self.assertEqual(i5.encoding, 0xE031BA95)

    def test6(self):
        i6 = Multiply('MLA R2,R10,R3, R12')
        i6.parse_line()
        i6.encode()
        self.assertEqual(i6.encoding, 0xE022C39A)

if __name__ == '__main__':
    unittest.main()
