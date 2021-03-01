import subprocess
import unittest
import difflib
import os
import pathlib

"""
Module dm_testbench collects functions to handle patterns for the data master testbench.
"""
class DmTestbench(unittest.TestCase):

  def startpattern(self, data_master, pattern_file):
    """
    Connect to the given data master and load the pattern file (dot format).
    The data master is halted, cleared, and statistics is reset.
    Search for the first pattern in the data master with 'dm-sched' and start it.
    """
    self.startAllPattern(data_master, pattern_file, onePattern=True)

  def startAllPattern(self, data_master, pattern_file, onePattern=False):
    """
    Connect to the given data master and load the pattern file (dot format).
    The data master is halted, cleared, and statistics is reset.
    Search for all pattern in the data master with 'dm-sched' and start these.
    """
    print (f"Connect to device '{data_master}', pattern file '{pattern_file}'.   ", end='', flush=True)
    process = subprocess.Popen(['dm-cmd', data_master, 'halt'])
    process.wait()
    self.assertEqual(process.returncode, 0, f'wrong return code {process.returncode}, Command line: dm-cmd {data_master} halt')
    process = subprocess.Popen(['dm-sched', data_master, 'clear'])
    process.wait()
    self.assertEqual(process.returncode, 0, f'wrong return code {process.returncode}, Command line: dm-sched {data_master} clear')
    process = subprocess.Popen(['dm-cmd', data_master, 'cleardiag'])
    process.wait()
    self.assertEqual(process.returncode, 0, f'wrong return code {process.returncode}, Command line: dm-cmd {data_master} cleardiag')
    process = subprocess.Popen(['dm-sched', data_master, 'add', pattern_file])
    process.wait()
    self.assertEqual(process.returncode, 0, f'wrong return code {process.returncode}, Command line: dm-sched {data_master} add {pattern_file}')
    # run 'dm-sched data_master' as a sub process.
    process = subprocess.Popen(['dm-sched', data_master], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    lines = stdout.decode('utf-8').splitlines()
    # start the first pattern found in the data master.
    patterns = False
    for i in range(len(lines)):
      if patterns and len(lines[i]) > 0:
        # print (f"Pattern: \n{lines[i]} {lines}")
        process = subprocess.Popen(['dm-cmd', data_master, 'startpattern', lines[i]])
        process.wait()
        self.assertEqual(process.returncode, 0, f'wrong return code {process.returncode}, Command line: dm-cmd {data_master} startpattern {lines[i]}')
        if onePattern:
          patterns = False
      if lines[i] == 'Patterns:':
        patterns = True

  def startAndCheckSubprocess(self, binary, command, parameter1, options='', expectedReturnCode=-1, linesCout=-1, linesCerr=-1):
    """
    Common method to start a subprocess and check the return code, .
    Start dm-cmd and dm-sched for the test steps with the arguments and check the output on stdout and stderr and the return code as well.
    """
    # pass cmd and args to the function
    if len(options) > 0:
      process = subprocess.Popen([binary, command, parameter1, options], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    else:
      process = subprocess.Popen([binary, command, parameter1], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    if expectedReturnCode > -1:
      self.assertEqual(process.returncode, expectedReturnCode, f'wrong return code {process.returncode}, Command line: {binary} {command} {parameter1} {options}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {binary} {command} {parameter1} {options}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {binary} {command} {parameter1} {options}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')

  def compareExpectedResult(self, file_current, file_expected, exclude):
    with open(file_current) as f_current:
      current = f_current.readlines()
    if len(exclude) > 0:
      current = [ x for x in current if exclude not in x ]
  #  print(f'current: {current}')
    with open(file_expected) as f_expected:
      expected = f_expected.readlines()
    diffLines = list(difflib.unified_diff(current, expected))
  #  print(f'Diff: {diffLines}')
    self.assertEqual(len(diffLines), 0, f'Diff: {diffLines}')

  def deleteFile(self, file_name):
    file_to_rem = pathlib.Path(file_name)
    file_to_rem.unlink()
