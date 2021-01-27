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

  def test_1print_args(self):
    print(f'Binary: {self.binary}.')

  def callScheduleCompare(self, file1, file2, options):
    """
    Common method for test cases: run scheduleCompare.
    Start scheduleCompare with the arguments and check the output on stdout and stderr and the return code as well.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([self.binary, file1, file2, options], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    print(f'Returncode: {process.returncode} of {self.binary}.')
    lines = stderr.decode('utf-8').splitlines()
    print(f'Lines:\n{lines}')
    test_result = False
    for line in lines:
      if 'Target node is NULL, target missing.' in line:
        test_result = True
    self.assertTrue(test_result, f'stderr: {lines}')
    self.assertEqual(len(lines), 1)

  def test_first_isomorphism(self):
    self.callScheduleCompare('test0.dot', 'test0.dot', '-v')


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
