#!/usr/bin/python3
import unittest
from armasm.instructions import SingleDataTransfer

class TestSingleDataTransfer(unittest.TestCase):
    def test1(self):
        i1 = SingleDataTransfer('LDRLTB R11, [R2]')
        i1.parse_line()
        i1.encode()
        self.assertEqual(i1.encoding, 0xB5D2B000)

    def test2(self):
        i2 = SingleDataTransfer('LDRB R2, [R3, #-20]!')
        i2.parse_line()
        i2.encode()
        self.assertEqual(i2.encoding, 0xE5732014)

    def test3(self):
        i3 = SingleDataTransfer('LDR R13, [R5, R2, LSL #31]')
        i3.parse_line()
        i3.encode()
        self.assertEqual(i3.encoding, 0xE795DF82)

    def test4(self):
        i4 = SingleDataTransfer('STRGTBT R12, [ R7] , -R1, ROR  #4')
        i4.parse_line()
        i4.encode()
        self.assertEqual(i4.encoding, 0xC667C261)

    def test5(self):
        i5 = SingleDataTransfer('STRBT  R5, [R8] ,  R1')
        i5.parse_line()
        i5.encode()
        self.assertEqual(i5.encoding, 0xE6E85001)

    def test6(self):
        i6 = SingleDataTransfer('STR  R11, [ R3 ], #245')
        i6.parse_line()
        i6.encode()
        self.assertEqual(i6.encoding, 0xE483B0F5)

    def test7(self):
        i7 = SingleDataTransfer('STR  R11, [ R3 LSL R4]')
        i7.parse_line()
        i7.encode()
        self.assertEqual(i7.encoding, 0xE483B0F5)

if __name__ == '__main__':
    unittest.main()

