#! /usr/bin/env python3

import unittest
import sys
import pathlib
import dm_testbench
import subprocess
#import time
#import signal
#import csv
#import contextlib
import datetime
#import collections

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

  def t1est_safe2remove_blockalign1(self):
    self.safe2removeTestcase('blockalign1', 'PPS1_TEST')
    
  def doBranch1(self):
    output = self.startAndGetSubprocessStdout(('dm-cmd', self.datamaster, 'deadline'), expectedReturnCode=0)
    print()
    print(f'Result deadline: {output}')
    microseconds = float(output[0][21:-3])
    deadlineTime = datetime.datetime.fromtimestamp(microseconds/1000000.0)
    print(f'{output[0][21:-3]}, {deadlineTime}')
    deadlineTime1 = datetime.datetime.fromtimestamp(microseconds/1000000.0 + 1.0)
    print(f'{output[0][21:-3]}, {deadlineTime1}')
    offsetTime = (deadlineTime1-datetime.datetime(1970,1,1)).total_seconds()
    offsetNanoseconds = offsetTime * 1000000000
    offsetNanosecondsStr = "{:.0f}".format(offsetNanoseconds)
    print(f'{offsetTime}, {offsetNanoseconds} {offsetNanoseconds:.0f}, {offsetNanosecondsStr}')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '10', '-a', '-l', offsetNanosecondsStr, 'Block1', 'B1'), expectedReturnCode=0)
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '10', '-a', '-l', offsetNanosecondsStr, 'Block1', 'C1'), expectedReturnCode=0)
  
  def test_branch1(self):
    self.startAllPattern(self.datamaster, 'branch1.dot')
    file_name = 'snoop_branch1.csv'
    parameter_column = 20
    self.snoopToCsvWithAction(file_name, self.doBranch1, 2)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_safe2remove_blockflow1(self):
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
