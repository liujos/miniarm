#!/usr/bin/python3
import unittest
from armasm.instructions import DataProcessing

class TestDataProcessing(unittest.TestCase):
    def test1(self):
        i1 = DataProcessing('ANDEQS   R0,R1, R2')
        i1.parse_line()
        i1.encode()
        self.assertEqual(i1.encoding, 0x00110002)

    def test2(self):
        i2 = DataProcessing('EORNE    R3,R4,R5, LSL #0')
        i2.parse_line()
        i2.encode()
        self.assertEqual(i2.encoding, 0x10243005)

    def test3(self):
        i3 = DataProcessing('SUBCSS R5, R6,   R7,LSR #32')
        i3.parse_line()
        i3.encode()
        self.assertEqual(i3.encoding, 0x20565027)

    def test4(self):
        i4 = DataProcessing('RSBCC  R8,  R9, R10, ASR #32')
        i4.parse_line()
        i4.encode()
        self.assertEqual(i4.encoding, 0x3069804A)

    def test5(self):
        i5 = DataProcessing('ADDMI R11,R12,R13,RRX')
        i5.parse_line()
        i5.encode()
        self.assertEqual(i5.encoding, 0x408CB06D)

    def test6(self):
        i6 = DataProcessing('ADCPL R11, R12, R2,ROR #0')
        i6.parse_line()
        i6.encode()
        self.assertEqual(i6.encoding, 0x50ACB002)

    def test7(self):
        i7 = DataProcessing('SBCVSS R10, R12,   #324')
        i7.parse_line()
        i7.encode()
        self.assertEqual(i7.encoding, 0x62DCAF51)

    def test8(self):
        i8 = DataProcessing('RSCVCS R1, R12,   R11, LSR R2')
        i8.parse_line()
        i8.encode()
        self.assertEqual(i8.encoding, 0x70FC123B)

    def test9(self):
        i9 = DataProcessing('TSTHI   R3, #245')
        i9.parse_line()
        i9.encode()
        self.assertEqual(i9.encoding, 0x831300F5)

    def test10(self):
        i10 = DataProcessing('TEQLS R10, R11')
        i10.parse_line()
        i10.encode()
        self.assertEqual(i10.encoding, 0x913A000B)

    def test11(self):
        i11 = DataProcessing('CMPGE R5, #4')
        i11.parse_line()
        i11.encode()
        self.assertEqual(i11.encoding, 0xA3550004)

    def test12(self):
        i12 = DataProcessing('CMNLT R1, R3, ASR #21')
        i12.parse_line()
        i12.encode()
        self.assertEqual(i12.encoding, 0xB1710AC3)

    def test13(self):
        i13 = DataProcessing('ORRGT R1, R2, R3, LSR R12')
        i13.parse_line()
        i13.encode()
        self.assertEqual(i13.encoding, 0xC1821C33)

    def test14(self):
        i14 = DataProcessing('BICLES   R1, R2, R3, ASR R3')
        i14.parse_line()
        i14.encode()
        self.assertEqual(i14.encoding, 0xD1D21353)

    def test15(self):
        i15 = DataProcessing('MVNS R10, R11, ROR R3')
        i15.parse_line()
        i15.encode()
        self.assertEqual(i15.encoding, 0xE1F0A37B)


if __name__ == '__main__':
    unittest.main()
        
