#! /usr/bin/env python3

import CommandsHistory as C
import pathlib
import shutil
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

"""Unit tests for CommandsHistory.py (rewrite datamaster log files into shell scripts).
Usage for tests:
python3 -m unittest testCommandsHistory.py -v
Add folder with CommandsHistory.py to the PYTHONPATH.
"""
class TestCommandsHistory(unittest.TestCase):

  def deleteFile(self, file_name):
    """Delete file <file_name>.
    """
    file_to_rem = pathlib.Path(file_name)
    file_to_rem.unlink()

  def runCommandsHistoryTest(self, history_file, begin_date, begin_time, end_date, end_time):
    with open('expected_' + history_file[9:] + '_stdout.txt') as expected_stdout:
      expectedStdout = expected_stdout.read().splitlines()
    with Capturing() as stdOutLines:
      C.main([history_file + '.log', begin_date, begin_time, end_date, end_time])
    self.assertEqual(stdOutLines, expectedStdout)
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

  def test_CommandsHistory_firstPart(self):
    """Test CommandHistory.py with a log file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021. Time interval ends before log file.
    The interval starts before the log file starts and ends before the end of the log file.
    This means that the first part of the log file is rewritten to a script.
    The output (stdout, generated script) is compared with the expected outut.
    """
    self.runCommandsHistoryTest('GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021', '2021-02-05', '12:00', '2021-02-05', '13:50')

  def test_CommandHistory_all(self):
    """Test CommandHistory.py with a log file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021_a. Time interval includes the whole log file.
    The interval starts before the log file starts and ends after the end of the log file.
    This means that the complete log file is rewritten to a script.
    The output (stdout, generated script) is compared with the expected outut.
    """
    self.runCommandsHistoryTest('GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021_a', '2021-02-05', '12:00', '2021-02-05', '14:50')

  def test_CommandHistory_inside(self):
    """Test CommandHistory.py with a log file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021_b. Time interval is inside the time span of log file.
    The interval starts after the log file starts and ends before the end of the log file.
    This means that a part inside the log file is rewritten to a script.
    The output (stdout, generated script) is compared with the expected outut.
    """
    self.runCommandsHistoryTest('GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021_b', '2021-02-05', '13:40', '2021-02-05', '13:50')

  def test_CommandHistory_lastPart(self):
    """Test CommandHistory.py with a log file GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021_c. Time interval starts after log file.
    The interval starts after the log file starts and ends after the end of the log file.
    This means that the last part of the log file is rewritten to a script.
    The output (stdout, generated script) is compared with the expected outut.
    """
    self.runCommandsHistoryTest('GeneratorCommandsHistory_Fri_Feb__5_13.33.22_2021_c', '2021-02-05', '13:40', '2021-02-05', '14:50')
