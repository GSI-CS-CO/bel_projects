#! /usr/bin/env python3

import unittest
import sys
import pathlib
import dm_testbench
import subprocess
#import time
#import signal
import csv
import contextlib
import datetime
import collections

"""
Class collects unit tests for safe2remove.
Main test case is:
Clear data master
Add schedule
Start all patterns
Check removal of one pattern, should fail while pattern is running.
Abort pattern
Check removal of this pattern, should be valid, since pattern is not running.
Remove this pattern.
Check status of remaining schedule.

Usage: ./unitTestSafe2Remove.py <datamaster>
"""
class UnitTestSafe2Remove(dm_testbench.DmTestbench):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.datamaster = test_datamaster

  def safe2removeTestcase(self, dot_file1, pattern_to_remove):
    self.startAllPattern(self.datamaster, dot_file1 + '.dot')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'chkrem', pattern_to_remove), expectedReturnCode=0)
    self.compareExpectedResult('debug.dot', dot_file1 + '-forbidden.dot', 'Created')
    self.deleteFile('debug.dot')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'abortpattern', pattern_to_remove), expectedReturnCode=0)
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'chkrem', pattern_to_remove), expectedReturnCode=0)
    self.compareExpectedResult('debug.dot', dot_file1 + '-safe.dot', 'Created')
    self.deleteFile('debug.dot')
    self.startAndCheckSubprocess(('dm-sched', self.datamaster, 'remove', dot_file1 + '-remove.dot'), expectedReturnCode=0)
    self.startAndCheckSubprocess(('dm-sched', self.datamaster, 'status', '-ostatus.dot'), expectedReturnCode=0)
    self.compareExpectedResult('status.dot', dot_file1 + '-status.dot', '')
    self.deleteFile('status.dot')

  def test_safe2remove_blockalign1(self):
    self.safe2removeTestcase('blockalign1', 'PPS1_TEST')
    
  def doBranch1(self):
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '100', 'Block1', 'B1'), expectedReturnCode=0)
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '100', 'Block1', 'C1'), expectedReturnCode=0)
  
  def test_branch1(self):
    self.startAllPattern(self.datamaster, 'branch1.dot')
    file_name = 'snoop_branch1.csv'
    parameter_column = 20
    self.snoopToCsvWithAction(file_name, self.doBranch1, 3)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def test_safe2remove_blockflow1(self):
    self.startAllPattern(self.datamaster, 'block-flow1.dot')
    file_name = 'snoop_block-flow1.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 5.0)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

if __name__ == '__main__':
  if len(sys.argv) > 1:
    test_datamaster = sys.argv.pop()
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)
