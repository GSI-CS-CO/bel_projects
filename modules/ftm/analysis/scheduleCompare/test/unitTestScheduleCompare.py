#!/usr/bin/python

import unittest
import subprocess
import sys
import os

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
    if expectedReturnCode > -1:
      self.assertEqual(process.returncode, expectedReturnCode, 
        f'wrong return code {process.returncode}, Command line: {self.binary} {file1} {file2} {options}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {self.binary} {file1} {file2} {options}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {self.binary} {file1} {file2} {options}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')

  def allPairsFilesInfolderTest(self, folder):
    files = os.listdir(folder)
    # print (files)
    counter = 0
    for dotFile1 in files:
      for dotFile2 in files:
        counter += 1
        if counter % 100 == 0:
          print(f'{counter},', end='', flush=True)
        if counter % 1000 == 0:
          print(f'', flush=True)
        if dotFile1 == dotFile2:
          returncode = 0
        else:
          returncode = 1
        self.callScheduleCompare(folder + dotFile1, folder + dotFile2, '-s', expectedReturnCode=returncode, linesCout=0)
    print(f'Pairs tested: {counter}. ', end='', flush=True)

  def allFilesInfolderTest(self, folder):
    files = os.listdir(folder)
    # print (files)
    counter = 0
    for dotFile1 in files:
      counter += 1
      if counter % 100 == 0:
        print(f'{counter},', end='', flush=True)
      if counter % 1000 == 0:
        print(f'', flush=True)
      self.callScheduleCompare(folder + dotFile1, '-t', expectedReturnCode=16, linesCout=1)
    print(f'Files tested: {counter}. ', end='', flush=True)

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
    self.callScheduleCompare('', '', '-h', expectedReturnCode=14, linesCerr=21, linesCout=0)

  def test_folder_dot_tmsg(self):
    self.allPairsFilesInfolderTest('dot_tmsg/')

  def test_folder_dot_block(self):
    self.allPairsFilesInfolderTest('dot_block/')

  def test_folder_dot_flow(self):
    self.allPairsFilesInfolderTest('dot_flow/')

  def test_folder_dot_flush(self):
    self.allPairsFilesInfolderTest('dot_flush/')

  def test_folder_dot_switch(self):
    self.allPairsFilesInfolderTest('dot_switch/')

  def test_folder_dot_wait(self):
    self.allPairsFilesInfolderTest('dot_wait/')

  def test_folder_dot_graph_entries(self):
    self.allPairsFilesInfolderTest('dot_graph_entries/')
    
  def test_dot_graph_entries_2(self):
    self.callScheduleCompare('dot_graph_entries/graph-entry-008600.dot', 'dot_graph_entries_2/graph-entry-009852.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries/graph-entry-008600.dot', 'dot_graph_entries_2/graph-entry-008541.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries/graph-entry-010773.dot', 'dot_graph_entries_2/graph-entry-010745.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries/graph-entry-016159.dot', 'dot_graph_entries_2/graph-entry-016193.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries_2/pro_2020_11_24.dot', 'dot_graph_entries_2/pro_2020_11_24.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.allFilesInfolderTest('dot_graph_entries_2/')
  
  def test_dot_boolean(self):
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_x.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)

  def test_folder_dot(self):
    self.allPairsFilesInfolderTest('dot1/')

if __name__ == '__main__':
  if len(sys.argv) > 1:
#    print(f"Arguments: {sys.argv}")
    test_binary = sys.argv.pop()
#    print(f"Arguments: {sys.argv}, {len(sys.argv)}")
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)
