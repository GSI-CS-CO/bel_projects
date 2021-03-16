#! /usr/bin/env python3

import CommandsHistory as C
import pathlib
import shutil
import subprocess
import sys
import unittest
from io import StringIO 

"""
Capture stdout with a context manager in a list.
Used to capture stdout during a method call.
"""
class Capturing(list):
  def __enter__(self):
    self._stdout = sys.stdout
    sys.stdout = self._stringio = StringIO()
    return self
  def __exit__(self, *args):
    self.extend(self._stringio.getvalue().splitlines())
    del self._stringio    # free up some memory
    sys.stdout = self._stdout

"""
Usage:
python3 -m unittest testCommandsHistory.py -v
Add folder with CommandsHistory.py to the PYTHONPATH.
"""
class TestCommandsHistory(unittest.TestCase):

  def deleteFile(self, file_name):
    """Delete file <file_name>.
    """
    file_to_rem = pathlib.Path(file_name)
    file_to_rem.unlink()

  def test_CommandsHistoryX(self):
    """Test CommandHistory.py with a log file and a time interval.
    The interval starts before the log file starts and ends before the end of the log file.
    This means that the first part of the log file is rewritten to a script.
    The output (stdout, generated script) is compared with the expected outut.
    """
    history_file = 'GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021'
    with Capturing() as stdOutLines:
      C.main([history_file + '.log', '2021-02-04', '12:00', '2021-02-05', '13:50'])
    self.assertEqual(stdOutLines, ['Rewrite command history file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021.log to script CommandsHistory_Fri_Feb__5_13.33.22_2021.sh.', 
      'Schedules written to CommandsHistory_Fri_Feb__5_13.33.22_2021/graph-entry-n.dot, where n is the entry number.', 
      'Lines: 3416, dot files: 32, first entry: 2021-02-05 13:36:22, last entry: 2021-02-05 14:07:52.'])
    lines = []
    shell_file = history_file[9:] + '.sh'
    with open(shell_file) as script:
      lines = script.readlines()
    expectedLines = []
    with open('expected_' + shell_file) as expected:
      expectedLines = expected.readlines()
    self.assertEqual(lines, expectedLines)
    self.deleteFile(shell_file)
    shutil.rmtree(history_file[9:] + '/')

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
