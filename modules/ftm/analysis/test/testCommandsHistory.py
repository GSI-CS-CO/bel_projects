#! /usr/bin/env python3

import unittest
import subprocess
import CommandsHistory as C

"""
Usage:
python3 -m unittest testCommandsHistory.py -v
Add folder with CommandsHistory.py to the PYTHONPATH.
"""
class TestCommandsHistory(unittest.TestCase):
  def startAndGetSubprocessStdout(self, argumentsList, expectedReturnCode=-1, linesCout=-1, linesCerr=-1):
    """
    Common method to start a subprocess and check the return code.
    The <argumentsList> contains the binary to execute and all arguments in one list.
    Start the binary for the test step with the arguments and check the output on stdout and stderr and the return code as well.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([*argumentsList], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    if expectedReturnCode > -1:
      self.assertEqual(process.returncode, expectedReturnCode, f'wrong return code {process.returncode}, Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {argumentsList}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')
    return stdout.decode("utf-8").splitlines()

  def test_CommandHistory0(self):
    stdOutLines = self.startAndGetSubprocessStdout(('../CommandsHistory.py', 'GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log', '2021-02-04', '12:00', '2021-02-05', '13:50'), expectedReturnCode=0)
    # print(stdOutLines)
    self.assertEqual(stdOutLines, ['Rewrite command history file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log to script CommandsHistory_Fri_Feb__5_13.33.22_2021.sh.', 
      'Schedules written to CommandsHistory_Fri_Feb__5_13.33.22_2021/graph-entry-n.dot, where n is the entry number.', 
      'Lines: 3416, dot files: 32, first entry: 2021-02-05 13:36:22, last entry: 2021-02-05 14:07:52.'])

  def test_CommandHistory1(self):
    stdOutLines = self.startAndGetSubprocessStdout(('../CommandsHistory.py', 'GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log', '2021-02-04', '12:00', '2021-02-05', '14:50'), expectedReturnCode=0)
    # print(stdOutLines)
    self.assertEqual(stdOutLines, ['Rewrite command history file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log to script CommandsHistory_Fri_Feb__5_13.33.22_2021.sh.', 
      'Schedules written to CommandsHistory_Fri_Feb__5_13.33.22_2021/graph-entry-n.dot, where n is the entry number.', 
      'Lines: 3416, dot files: 37, first entry: 2021-02-05 13:36:22, last entry: 2021-02-05 14:07:52.'])

  def test_CommandHistory2(self):
    stdOutLines = self.startAndGetSubprocessStdout(('../CommandsHistory.py', 'GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log', '2021-02-04', '13:40', '2021-02-05', '13:50'), expectedReturnCode=0)
    # print(stdOutLines)
    self.assertEqual(stdOutLines, ['Rewrite command history file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log to script CommandsHistory_Fri_Feb__5_13.33.22_2021.sh.', 
      'Schedules written to CommandsHistory_Fri_Feb__5_13.33.22_2021/graph-entry-n.dot, where n is the entry number.', 
      'Lines: 3416, dot files: 32, first entry: 2021-02-05 13:36:22, last entry: 2021-02-05 14:07:52.'])

  def test_CommandHistory3(self):
    stdOutLines = self.startAndGetSubprocessStdout(('../CommandsHistory.py', 'GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log', '2021-02-04', '13:40', '2021-02-05', '14:50'), expectedReturnCode=0)
    # print(stdOutLines)
    self.assertEqual(stdOutLines, ['Rewrite command history file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log to script CommandsHistory_Fri_Feb__5_13.33.22_2021.sh.', 
      'Schedules written to CommandsHistory_Fri_Feb__5_13.33.22_2021/graph-entry-n.dot, where n is the entry number.', 
      'Lines: 3416, dot files: 37, first entry: 2021-02-05 13:36:22, last entry: 2021-02-05 14:07:52.'])
