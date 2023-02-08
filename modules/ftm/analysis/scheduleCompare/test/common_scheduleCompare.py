import unittest
import subprocess
import sys
import os
from multiprocessing.pool import ThreadPool

global test_binary
"""Class collects unit tests for scheduleCompare.
Tests run scheduleCompare with two dot files and check the result.
In addition, run scheduleCompare in test mode, comparing a dot file with itself.
"""
class CommonScheduleCompare(unittest.TestCase):
  @classmethod
  def setUpClass(self):
    """
    Set up for all test cases: store the environment variables in variables.
    """
    self.binary = os.environ.get('TEST_BINARY_SCHEDULECOMPARE', 'scheduleCompare')

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

  def filePairTask(self, dotFile1, dotFile2, folder):
    if dotFile1 == dotFile2:
      returncode = 0
    else:
      returncode = 1
    self.callScheduleCompare(folder + dotFile1, folder + dotFile2, '-s', expectedReturnCode=returncode, linesCout=0)
    return 1

  def allPairsFilesInFolderTest(self, folder):
    files = os.listdir(folder)
    # ~ print (files)
    files = [ x for x in files if '.dot' in x ]
    # ~ print (files)
    counter = 0
    filePairs = [(dotFile1, dotFile2, folder) for dotFile1 in files for dotFile2 in files]
    with ThreadPool() as pool:
      for result in pool.starmap(self.filePairTask, filePairs):
        counter += result
        if counter % 100 == 0:
          print(f'{counter},', end='', flush=True)
        if counter % 1000 == 0:
          print(f'', flush=True)
    print(f'Pairs tested: {counter}. ', end='', flush=True)

  def fileTask(self, dotFile1, folder):
    self.callScheduleCompare(folder + dotFile1, '-t', expectedReturnCode=17, linesCout=1)
    return 1

  def allFilesInFolderTest(self, folder):
    files = os.listdir(folder)
    files = [ x for x in files if '.dot' in x ]
    # print (files)
    counter = 0
    fileArgs = [(dotFile1, folder) for dotFile1 in files]
    with ThreadPool() as pool:
      for result in pool.starmap(self.fileTask, fileArgs):
        counter += result
        if counter % 100 == 0:
          print(f'{counter},', end='', flush=True)
        if counter % 1000 == 0:
          print(f'', flush=True)
    print(f'Files tested: {counter}. ', end='', flush=True)
