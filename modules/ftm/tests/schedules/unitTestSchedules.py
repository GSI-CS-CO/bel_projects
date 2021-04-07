#! /usr/bin/env python3

import sys
import dm_testbench
import unittest

"""
Start all pattern in a schedule and analyse the frequency of timing messages for 5 seconds.

Required argument:  device of data master.
"""
class Schedules(dm_testbench.DmTestbench):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.datamaster = test_datamaster

  def test_frequency_schedule1(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8 #20
    self.snoopToCsv(file_name, 6.0)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_frequency_schedule1_0060(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8
    self.snoopToCsv(file_name, 60.0)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_frequency_schedule1_0600(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8
    self.snoopToCsv(file_name, 600.0)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_frequency_schedule1_3600(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8
    self.snoopToCsv(file_name, 3600.0)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

if __name__ == '__main__':
  if len(sys.argv) > 1:
    test_datamaster = sys.argv.pop()
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)
