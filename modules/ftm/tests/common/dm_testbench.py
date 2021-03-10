import subprocess  # used in startAllPatterns
import unittest    # contains super class
import difflib     # used in compareExpectedResult
import os
import pathlib
import csv         # used in analyseFrequencyFromCsv
import contextlib
import datetime    # used in analyseFrequencyFromCsv
import collections # used in analyseFrequencyFromCsv

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

  def startAndCheckSubprocess(self, argumentsList, expectedReturnCode=-1, linesCout=-1, linesCerr=-1):
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

  def compareExpectedResult(self, file_current, file_expected, exclude):
    """
    Compare a file with a test result with an expected result contained in <file_expected>.
    Lines with the string <exclude> are removed from <file_current> before checking against <file_expected>.
    Assert that a unified diff has no lines.
    """
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

  def snoopToCsv(self, csv_file_name, duration=1.0):
    """
    Snoop timing messages with saft-ctl for <duration> seconds (default = 1.0).
    Write the messages to <csv_file_name>.
    Details: start saft-ctl with Popen, wait for <duration> seconds.
    When the TimeoutExpired exception pops up, kill saft-ctl and wait for at most 5 seconds.
    At least assert the return code -9 of saft-ctl.
    """
    with open(csv_file_name, 'wb') as file1:
      try:
        process = subprocess.Popen(['saft-ctl', 'x', '-fvx', 'snoop', '0', '0', '0'], stdout=file1)
        process.wait(duration)
      except subprocess.TimeoutExpired as excep:
        process.kill()
        process.wait(5)
        self.assertEqual(process.returncode, -9, f'Returncode: {process.returncode}')

  def snoopToCsvWithAction(self, csv_file_name, action, duration=1.0):
    """
    Snoop timing messages with saft-ctl for <duration> seconds (default = 1.0).
    Write the messages to <csv_file_name>.
    Details: start saft-ctl with Popen, wait for <duration> seconds.
    When the TimeoutExpired exception pops up, kill saft-ctl and wait for at most 5 seconds.
    At least assert the return code -9 of saft-ctl.
    """
    with open(csv_file_name, 'wb') as file1:
      try:
        process = subprocess.Popen(['saft-ctl', 'x', '-fvx', 'snoop', '0', '0', '0'], stdout=file1)
        action()
        process.wait(duration)
      except subprocess.TimeoutExpired as excep:
        process.kill()
        process.wait(5)
        self.assertEqual(process.returncode, -9, f'Returncode: {process.returncode}')

  def analyseFrequencyFromCsv(self, csv_file_name, column=20, printTable=True):
    """
    Analyse the frequency of the values in the specified column. Defaule column is 20 (parameter of the timing message).
    Prints (if printtable=True) the table of values, counters, and frequency over the whole time span.
    """
    line_count = 0
    maxTime = datetime.datetime.strptime("2000-01-01", '%Y-%m-%d')
    minTime = datetime.datetime.strptime("2500-01-01", '%Y-%m-%d')
    listParam = []
    # Read this file snoop_protocol.csv as csv
    with open(csv_file_name) as csv_file:
      csv_reader = csv.reader(csv_file, delimiter=' ')
      for row in csv_reader:
        line_count += 1
        time1 = datetime.datetime.strptime(row[1] + ' ' + row[2][:-3], '%Y-%m-%d %H:%M:%S.%f')
        if minTime > time1:
          minTime = time1
        if time1 > maxTime:
          maxTime = time1
        listParam.append(row[column])
      timeSpan = (maxTime-minTime).total_seconds()
      listCounter = sorted(collections.Counter(listParam).items())
      if printTable:
        print()
        print(f'{"Value":^20s}  {"Count":>6s}   {"Frequency":>9s}')
        for key, value in listCounter:
          print(f'{key:>20s}: {value:6d} {value/timeSpan:9.3f}Hz')
        print(f'{"All":>20s}: {line_count:6d} {line_count/timeSpan: >9.3f}Hz, time span: {timeSpan:0.6f}sec')

  def deleteFile(self, file_name):
    """
    Delete file <file_name>.
    """
    file_to_rem = pathlib.Path(file_name)
    file_to_rem.unlink()
