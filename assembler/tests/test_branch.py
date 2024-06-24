#!/usr/bin/python3
import unittest
from armasm.instructions import Branch

class TestBranch(unittest.TestCase):
    def setUp(self):
        self.symbol_table = {'loop' : 0, 
                             'end'  : 3,
                             'here' : 5,
                             'there': 10}

    def test1(self):
        i1 = Branch('BLNE   LOOP', self.symbol_table, 0)
        i1.encode()
        self.assertEqual(i1.encoding, 0x1BFFFFFE)

    @unittest.skip("Not implemented")
    def test2(self):
        i2 = Branch('BLT    END', self.symbol_table, 1)
        i2.encode()
        print(i2.encoding)
        self.assertEqual(i2.encoding, )

    @unittest.skip("Not implemented")
    def test3(self):
        i3 = Branch('BGT HERE', self.symbol_table, 2)
        i3.encode()
        self.assertEqual(i3.encoding, )

    @unittest.skip("Not implemented")
    def test4(self):
        i4 = Branch('B THERE', self.symbol_table, 20)
        i4.encode()
        self.assertEqual(i4.encoding, )

if __name__ == '__main__':
    unittest.main()
