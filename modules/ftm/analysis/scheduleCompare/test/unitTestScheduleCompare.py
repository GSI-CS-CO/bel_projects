#!/usr/bin/python

import unittest
import subprocess
import sys

global test_binary
"""
Class collects unit tests for scheduleCompare.
Tests run scheduleCompare with two dot files and check the result.

Usage: ./unittest/ScheduleCompare.py
"""
class TestScheduleCompare(unittest.TestCase):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.binary = test_binary

  def t1est_1print_args(self):
    print(f'Binary: {self.binary}.', end='')

  def callScheduleCompare(self, file1, file2, options='', expectedReturnCode=-1, linesCout=-1, linesCerr=-1):
    """
    Common method for test cases: run scheduleCompare.
    Start scheduleCompare with the arguments and check the output on stdout and stderr and the return code as well.
    """
    # pass cmd and args to the function
    if len(options) > 0:
      process = subprocess.Popen([self.binary, file1, file2, options], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    else:
      process = subprocess.Popen([self.binary, file1, file2], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    # print(f'Returncode: {process.returncode} of {self.binary}.')
    if expectedReturnCode > -1:
      self.assertEqual(process.returncode, expectedReturnCode, f'wrong return code {process.returncode}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'stderr: {lines}')
    # print(f'stderr Lines:\n{lines}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'stdout: {lines}')
    # print(f'stdout Lines:\n{lines}')

  def test_first_isomorphism(self):
    self.callScheduleCompare('test0.dot', 'test0.dot', expectedReturnCode=0, linesCerr=0, linesCout=3)

  def test_first_isomorphism_verbose(self):
    self.callScheduleCompare('test0.dot', 'test0.dot', '-v', expectedReturnCode=0, linesCerr=0, linesCout=25)

  def test_subgraph_isomorphism(self):
    self.callScheduleCompare('x0.dot', 'x1.dot', expectedReturnCode=2, linesCerr=0, linesCout=3)

  def test_subgraph_isomorphism_verbose(self):
    self.callScheduleCompare('x0.dot', 'x1.dot', '-v', expectedReturnCode=2, linesCerr=0, linesCout=31)

  def test_subgraph_isomorphism_superverbose(self):
    self.callScheduleCompare('x0.dot', 'x1.dot', '-vv', expectedReturnCode=2, linesCerr=0, linesCout=51)

  def test_subgraph_isomorphism_silent(self):
    self.callScheduleCompare('x0.dot', 'x1.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)

  def test_usage_message(self):
    self.callScheduleCompare('', '', '-h', expectedReturnCode=14, linesCerr=16, linesCout=0)


  def targetName(self, command, count_out=0, count_err=0, options=''):
    """
    Common method for commands with target name.
    Start dm-cmd with the command and check the output on stdout and stderr.
    The checks check only the number of lines of the output, not the content.
    """
    process = subprocess.Popen([self.binary, self.data_master, command, 'B_PPS', options], stderr=subprocess.PIPE, stdout=subprocess.PIPE)   # pass cmd and args to the function
    stdout, stderr = process.communicate()   # get command output and error
    lines = stdout.decode('utf-8').splitlines()
#    print(f'Lines:\n{lines}')
    self.assertEqual(len(lines), count_out, f'stdout, Number of lines {len(lines)}')
    lines = stderr.decode('utf-8').splitlines()
#    print(f'Lines:\n{lines}')
    self.assertEqual(len(lines), count_err, f'stderr, Number of lines {len(lines)}')

  def t1est_abswait(self):
    self.targetName('abswait', options='100')

if __name__ == '__main__':
  if len(sys.argv) > 1:
#    print(f"Arguments: {sys.argv}")
    test_binary = sys.argv.pop()
#    print(f"Arguments: {sys.argv}, {len(sys.argv)}")
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)
