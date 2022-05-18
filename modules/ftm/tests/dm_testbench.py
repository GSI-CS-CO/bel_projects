import collections # used in analyseFrequencyFromCsv
import contextlib
import csv         # used in analyseFrequencyFromCsv
import datetime    # used in analyseFrequencyFromCsv
import difflib     # used in compareExpectedResult
import os
import pathlib
import subprocess  # used in startAllPatterns
import sys
import threading
import time
import unittest    # contains super class

"""
Module dm_testbench collects functions to handle patterns for the data master testbench.
"""
class DmTestbench(unittest.TestCase):
  @classmethod
  def setUpClass(self):
    """Set up for all test cases: store the environment variables in variables.
    """
    self.binaryDmCmd = os.environ.get('TEST_BINARY_DM_CMD', 'dm-cmd')
    self.binaryDmSched = os.environ.get('TEST_BINARY_DM_SCHED', 'dm-sched')
    self.datamaster = os.environ['DATAMASTER']
    self.schedules_folder = os.environ.get('TEST_SCHEDULES', 'schedules/')
    self.snoop_command = os.environ.get('SNOOP_COMMAND', 'saft-ctl tr0 -xv snoop 0 0 0')
    self.patternStarted = False

  def setUp(self):
    self.initDatamaster()

  def initDatamaster(self):
    """Initialize (clean) the datamaster.
    The data master is halted, cleared, and statistics is reset.
    This is all done with 'dm-cmd reset all'.
    """
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'reset', 'all'])

  def addSchedule(self, scheduleFile):
    """Connect to the given data master and load the schedule file (dot format).
    """
    self.startAllPattern(scheduleFile, start=False)

  def startPattern(self, scheduleFile):
    """Connect to the given data master and load the schedule file (dot format).
    Search for the first pattern in the data master with self.binaryDmSched and start it.
    """
    self.startAllPattern(scheduleFile, '', onePattern=True)

  def startPattern(self, scheduleFile, pattern=''):
    """Connect to the given data master and load the schedule file (dot format).
    Start the given pattern.
    """
    self.startAllPattern(scheduleFile, pattern, onePattern=False)

  def startAllPattern(self, scheduleFile, pattern='', onePattern=False, start=True):
    """If a scheduleFile is given, connect to the given data master
    and load the schedule file (dot format).
    If onePattern=False and start=True, search for all pattern in the
    data master with self.binaryDmSched and start these.
    If onePattern is True, start only the first pattern in the scheduleFile.
    If start is False, do not start a pattern.
    """
    if len(scheduleFile) > 0:
      scheduleFile = self.schedules_folder + scheduleFile
      print (f"Connect to device '{self.datamaster}', schedule file '{scheduleFile}'.   ", end='', flush=True)
      self.startAndGetSubprocessStdout([self.binaryDmSched, self.datamaster, 'add', scheduleFile])
    if start:
      if len(pattern) > 0:
        self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'startpattern', pattern])
      else:
        # get the names of all pattern on this datamaster
        lines = self.startAndGetSubprocessStdout([self.binaryDmSched, self.datamaster])
        # start all pattern found in the data master.
        patterns = False
        for i in range(len(lines)):
          if patterns and len(lines[i]) > 0:
            # print (f"Pattern: \n{lines[i]} {lines} {lines[i].split()[0]}")
            self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'startpattern', lines[i].split()[0]])
            if onePattern:
              patterns = False
          if 'Patterns' in lines[i]:
            patterns = True

  def startAndCheckSubprocess(self, argumentsList, expectedReturnCode=[0], linesCout=-1, linesCerr=-1):
    """Common method to start a subprocess and check the return code.
    The <argumentsList> contains the binary to execute and all arguments in one list.
    Start the binary for the test step with the arguments and check the output on stdout and stderr and the return code as well.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([*argumentsList], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    self.assertTrue(process.returncode in expectedReturnCode, f'wrong return code {process.returncode}, expected: {expectedReturnCode}, '
          + f'Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {argumentsList}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')

  def startAndGetSubprocessStdout(self, argumentsList, expectedReturnCode=[0], linesCout=-1, linesCerr=-1):
    return self.startAndGetSubprocessOutput(argumentsList, expectedReturnCode, linesCout, linesCerr)[0]

  def startAndGetSubprocessOutput(self, argumentsList, expectedReturnCode=[-1], linesCout=-1, linesCerr=-1):
    """Common method to start a subprocess and check the return code.
    The <argumentsList> contains the binary to execute and all arguments in one list.
    Start the binary for the test step with the arguments and check the number of lines for stdout and stderr.
    Check that the return code is in a list of allowed return codes.
    Return both stdout, stderr as list of lines.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([*argumentsList], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    self.assertTrue(process.returncode in expectedReturnCode, f'wrong return code {process.returncode}, expected: {expectedReturnCode}, '
          + f'Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {argumentsList}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')
    return [stdout.decode("utf-8").splitlines(), stderr.decode("utf-8").splitlines()]

  def removePaintedFlags(self, dotLines):
    """Remove flags and fillcolors of painted nodes from dot string (parameter dotLines).
    """
    for index, line in enumerate(dotLines):
      dotLines[index] = line.replace('flags="0x00000107"', 'flags="0x00000007"') \
      .replace('flags="0x00002107"', 'flags="0x00002007"') \
      .replace('flags="0x00108107"', 'flags="0x00108007"') \
      .replace('flags="0x00708107"', 'flags="0x00708007"') \
      .replace('flags="0x00100107"', 'flags="0x00100007"') \
      .replace('fillcolor = "green"', 'fillcolor = "white"') \
      .replace('flags="0x00020007"', 'flags="0x00000007"') \
      .replace('flags="0x00020207"', 'flags="0x00000207"') \
      .replace('flags="0x00022007"', 'flags="0x00002007"') \
      .replace('flags="0x00120007"', 'flags="0x00100007"') \
      .replace('flags="0x00120207"', 'flags="0x00100207"') \
      .replace('flags="0x00128007"', 'flags="0x00108007"') \
      .replace(', fontname="Times-Bold", fontcolor = "blue2", fontsize="16"', '')
    return dotLines

  def compareExpectedResult(self, fileCurrent, fileExpected, exclude=''):
    """Compare a file with a test result with an expected result contained in <fileExpected>.
    Lines with the string <exclude> are removed from <fileCurrent> before checking against <fileExpected>.
    Assert that a unified diff has no lines.
    """
    with open(fileCurrent, 'r') as f_current:
      current = f_current.readlines()
    if len(exclude) > 0:
      current = [ x for x in current if exclude not in x ]
    current = self.removePaintedFlags(current)
    with open(fileExpected, 'r') as f_expected:
      expected = f_expected.readlines()
    diffLines = list(difflib.unified_diff(current, expected, n=0))
    self.assertEqual(len(diffLines), 0, f'Diff: file {fileExpected}\n{diffLines}')

  def compareExpectedOutput(self, output, fileExpected, exclude='', excludeField='', delete=[]):
    """Compare an output with a test result with an expected result contained in <fileExpected>.
    1. Lines with the string <exclude> are removed from <output> before checking against <fileExpected>.
    2. Lines in the list 'delete' are deleted in <output> and <fileExpected>.
    3. Lines which contain <excludeField> are not compared after the string specified in 'excludeField'.
    Assert that a unified diff has no lines.
    """
    current = output
    if len(exclude) > 0:
      current = [ x for x in current if exclude not in x ]
    for index in delete:
      del current[index]
    if len(excludeField) > 0:
      for line in current:
        index = current.index(line)
        pos = line.find(excludeField)
        if pos > -1:
          current[index] = line[:(pos + len(excludeField))]
    current = self.removePaintedFlags(current)
    # ~ with open('compare-current.txt', 'w') as file1:
      # ~ file1.write("\n".join(current))
    with open(fileExpected, 'r') as f_expected:
      expected = f_expected.read().splitlines()
    for index in delete:
      del expected[index]
    if len(excludeField) > 0:
      for line in expected:
        index = expected.index(line)
        pos = line.find(excludeField)
        if pos > -1:
          expected[index] = line[:(pos + len(excludeField))]
    # ~ with open('compare-expected.txt', 'w') as file2:
      # ~ file2.write("\n".join(expected))
    diffLines = list(difflib.unified_diff(current, expected, n=0))
    self.assertEqual(len(diffLines), 0, f'Diff: file {fileExpected}\n{diffLines}')

  def getSnoopCommand(self, duration):
    snoop_command1 = self.snoop_command + ' ' + str(duration)
    if self.snoop_command[len(self.snoop_command)-1:] == "'":
      snoop_command1 = self.snoop_command[:-1] + ' ' + str(duration) + "'"
    # ~ print(f'getSnoopCommand: {snoop_command1}')
    return snoop_command1

  def snoopToCsv(self, csvFileName, duration=1):
    """Snoop timing messages with saft-ctl for <duration> seconds (default = 1) and write the messages to <csvFileName>.
    Details: start saft-ctl with Popen, run it for <duration> seconds.
    """
    with open(csvFileName, 'wb') as file1:
      process = subprocess.run(self.getSnoopCommand(duration), shell=True, check=True, stdout=file1)
      self.assertEqual(process.returncode, 0, f'Returncode: {process.returncode}')

  def snoopToCsvWithAction(self, csvFileName, action, duration=1):
    """Snoop timing messages with saft-ctl for <duration> seconds (default = 1).
    Write the messages to <csvFileName>.
    Details: start saft-ctl with Popen in its own thread, run it for <duration> seconds.
    action should end before snoop.
    """
    snoop = threading.Thread(target=self.snoopToCsv, args=(csvFileName, duration))
    snoop.start()
    action()
    snoop.join()

  def analyseFrequencyFromCsv(self, csvFileName, column=20, printTable=True, checkValues=dict()):
    """Analyse the frequency of the values in the specified column. Default column is 20 (parameter of the timing message).
    Prints (if printTable=True) the table of values, counters, and frequency over the whole time span.
    Column for EVTNO is 8.
    checkValues is a dictionary of key-value pairs to check. Key is a value in the column and value is the required frequency.
    The value can be '>n', '<n', '=n', 'n' (which is the same as '=n').
    Example: column=8 and checkValues={('0x0001', 62)} checks that EVTNO 0x0001 occurs in 62 lines of the file to analyse.
    Example: column=8 and checkValues={('0x0002', '>0')} checks that EVTNO 0x0002 occurs at least once in the file to analyse.
    """
    line_count = 0
    maxTime = datetime.datetime.strptime("2000-01-01", '%Y-%m-%d')
    minTime = datetime.datetime.strptime("2500-01-01", '%Y-%m-%d')
    listParam = []
    # Read csvFileName as csv and collect the column in listParam.
    # Read the message timestamp from column 1 and 2, precision: microseconds, not nanoseconds.
    with open(csvFileName) as csv_file:
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
      listCounter = dict(sorted(collections.Counter(listParam).items()))
      if printTable:
        maxLengthKey = len("Value")
        maxLengthValue = len("Count")
        if len(str(line_count)) > maxLengthValue:
          maxLengthValue = len(str(line_count))
        alignKeys = False
        for key in listCounter:
          value = listCounter[key]
          if len(key) > maxLengthKey:
            maxLengthKey = len(key)
          if len(str(value)) > maxLengthValue:
            maxLengthValue = len(str(value))
          if "!delayed" in key or "!conflict" in key:
            alignKeys = True
        print()
        print(f'{"Value":^{maxLengthKey+1}s}  {"Count":>{maxLengthValue}s}   {"Frequency":>9s}')
        for key in listCounter:
          value = listCounter[key]
          keyAligned = key
          if "!" not in key and alignKeys:
            keyAligned = key + "         "
          if "!delayed" in key:
            keyAligned = key + " "
          if timeSpan > 0:
            print(f'{keyAligned:>{maxLengthKey + 1}s}: {value:{maxLengthValue}d} {value/timeSpan:9.3f}Hz')
          else:
            print(f'{keyAligned:>{maxLengthKey + 1}s}: {value:{maxLengthValue}d}')
        if timeSpan > 0:
          print(f'{"All":>{maxLengthKey + 1}s}: {line_count:{maxLengthValue}d} {line_count/timeSpan: >9.3f}Hz, time span: {timeSpan:0.6f}sec')
        else:
          print(f'{"All":>{maxLengthKey + 1}s}: {line_count:{maxLengthValue}d}, time span: {timeSpan:0.6f}sec')
      if len(checkValues) > 0:
        for item in checkValues:
          try:
            if str(checkValues[item])[0] == '>':
              self.assertGreater(listCounter[item], int(checkValues[item][1:]), f'assertGreater: is:{listCounter[item]} expected:{checkValues[item]}')
            elif str(checkValues[item])[0] == '<':
              self.assertGreater(int(checkValues[item][1:]), listCounter[item], f'assertSmaller: is:{listCounter[item]} expected:{checkValues[item]}')
            elif str(checkValues[item])[0] == '=':
              self.assertEqual(listCounter[item], int(checkValues[item][1:]), f'assertEqual: is:{listCounter[item]} expected:{checkValues[item]}')
            else:
              self.assertEqual(listCounter[item], int(checkValues[item]), f'assertEqual: is:{listCounter[item]} expected:{checkValues[item]}')
          except KeyError as keyError:
            self.fail(f'KeyError: expected value not found {keyError}')
          except:
            raise

  def analyseDmCmdOutput(self, threadsToCheck=0):
    outputStdoutStderr = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster), [0])
    output = outputStdoutStderr[0]
    msgCounts = {}
    # ~ firstCount = 0
    threadsCheck = 0
    index = 0
    countLines = len(output)
    offset = 0
    offset1 = 0
    cpu = 0
    thread = 0
    running = ''
    for line in output:
      if index > 11 and index < (countLines - 1):
        try:
          cpu = int(line[3 + offset1])
          thread = int(line[9 + offset1])
          running = line[21 + offset1:24 + offset1]
          count = int(line[32 + offset:42 + offset])
        except ValueError:
          print(f'ValueError:{index} CPU {cpu} Thread {thread}  line: "{line}", "{line[3 + offset1]}", "{line[9 + offset1]}", "{line[32 + offset:42 + offset]}", offset={offset}, offset1={offset1}')
        msgCounts[str(10*cpu) + str(thread)] = str(count)
        if running == 'yes':
          threadsCheck = threadsCheck + (1 << (8*cpu + thread))
        # ~ print(f'threadsCheck: {threadsCheck:#X}, threadsToCheck: {threadsToCheck:#X}, running:{running} {cpu} {thread}')
        if offset == 0:
          offset = 10
          offset1 = 5
        else:
          offset = 0
          offset1 = 0
      index = index + 1
    self.assertEqual(countLines-13, len(msgCounts))
    if threadsToCheck > 0:
      self.assertEqual(threadsCheck, threadsToCheck, f'threads running: {threadsCheck:#X}, expected: {threadsToCheck:#X}')
    return msgCounts

  def checkRunningThreadsCmd(self, messageInterval=1.0):
    firstCounts = self.analyseDmCmdOutput()
    self.delay(messageInterval)
    secondCounts = self.analyseDmCmdOutput()
    for key in firstCounts:
      firstCount = int(firstCounts[key])
      secondCount = int(secondCounts[key])
      # ~ print(f'key={key}, firstCount={firstCount}, secondCount={secondCount}')
      cpu = int(key[0])
      thread = int(key[1])
      self.assertGreater(secondCount, firstCount, f'CPU {cpu} Thread {thread} First: {firstCount}, second: {secondCount}')
      self.assertGreater(firstCount, 0, f'CPU {cpu} Thread {thread} firstCount is {firstCount}')

  def delay(self, duration):
    """Delay for <duration> seconds. <duration> is a float.
    """
    time.sleep(duration)

  def deleteFile(self, fileName):
    """Delete file <fileName>.
    """
    fileToRemove = pathlib.Path(fileName)
    if fileToRemove.exists():
      fileToRemove.unlink()
