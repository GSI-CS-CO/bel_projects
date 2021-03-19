#! /usr/bin/env python3

import dm_testbench
import sys
import unittest

global test_datamaster
"""
Start a pps pattern.

Required argument: device of data master.
"""
class DmPps(dm_testbench.DmTestbench):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.datamaster = test_datamaster

  def test_pps(self):
    file_test_pattern = 'pps2.dot'
    self.startPattern(self.datamaster, file_test_pattern)

if __name__ == '__main__':
  if len(sys.argv) > 1:
    test_datamaster = sys.argv.pop()
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)

