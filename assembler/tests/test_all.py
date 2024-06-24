import unittest
from test_single_data_transfer import TestSingleDataTransfer
from test_multiply import TestMultiply
from test_data_processing import TestDataProcessing

def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(TestSingleDataTransfer))
    suite.addTest(unittest.makeSuite(TestMultiply))
    suite.addTest(unittest.makeSuite(TestDataProcessing))
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner()
    runner.run(suite())


