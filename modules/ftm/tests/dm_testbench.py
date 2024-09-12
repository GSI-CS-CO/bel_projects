import collections # used in analyseFrequencyFromCsv
import contextlib
import csv         # used in analyseFrequencyFromCsv
import datetime    # used in analyseFrequencyFromCsv
import difflib     # used in compareExpectedResult
import logging
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
    self.schedulesFolder = os.environ.get('TEST_SCHEDULES', 'schedules/')
    self.snoop_command = os.environ.get('SNOOP_COMMAND', 'saft-ctl tr0 -xv snoop 0 0 0')
    self.deleteFiles = os.environ.get('DELETE_FILES', 'True')
    self.patternStarted = False
    self.cpuQuantity, self.threadQuantity = self.getThreadQuantityFromFirmware()

  def setUp(self):
    # ~ self.logToFile('    setUp:' + str(datetime.datetime.now()), 'dm-test-timestamps.txt')
    self.initDatamaster()

  # ~ def tearDown(self):
    # ~ self.logToFile(' tearDown:' + str(datetime.datetime.now()), 'dm-test-timestamps.txt')

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
      scheduleFile = self.schedulesFolder + scheduleFile
      # ~ print (f"Connect to device '{self.datamaster}', schedule file '{scheduleFile}'.   ", end='', flush=True)
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
    self.startAndGetSubprocessOutput(argumentsList, expectedReturnCode, linesCout, linesCerr)

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
      .replace('flags="0x00002102"', 'flags="0x00002002"') \
      .replace('flags="0x00002107"', 'flags="0x00002007"') \
      .replace('flags="0x00008107"', 'flags="0x00008007"') \
      .replace('flags="0x00108107"', 'flags="0x00108007"') \
      .replace('flags="0x00708107"', 'flags="0x00708007"') \
      .replace('flags="0x00100107"', 'flags="0x00100007"') \
      .replace('fillcolor = "green"', 'fillcolor = "white"') \
      .replace('flags="0x00020007"', 'flags="0x00000007"') \
      .replace('flags="0x00020207"', 'flags="0x00000207"') \
      .replace('flags="0x0002a207"', 'flags="0x0000a207"') \
      .replace('flags="0x0002a208"', 'flags="0x0000a208"') \
      .replace('flags="0x0003a207"', 'flags="0x0001a207"') \
      .replace('flags="0x00022002"', 'flags="0x00002002"') \
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

  def getSnoopCommand(self, eventId='0', mask='0', duration=1):
    snoop_command1 = self.snoop_command.replace('snoop 0 0 0', f'snoop {eventId} {mask} 0 {str(duration)}')
    # ~ print(f'getSnoopCommand: {snoop_command1}')
    return snoop_command1

  def getEbResetCommand(self):
    """Get the eb-reset command. If eb-reset exists in the repository or
    workspace, this file name is returned. Otherwise 'eb-reset' is returned,
    which assumes that eb-reset is installed.
    """
    ebResetCommand1 = '../../../tools/eb-reset'
    if not os.path.isfile(ebResetCommand1):
      ebResetCommand1 = 'eb-reset'
    # ~ print(f'getEbResetCommand: {ebResetCommand1}')
    return ebResetCommand1

  def snoopToCsv(self, csvFileName, eventId='0', mask='0', duration=1, resource=None):
    """Snoop timing messages with saft-ctl for <duration> seconds (default = 1) and write the messages to <csvFileName>.
    Details: start saft-ctl with Popen, run it for <duration> seconds.
    Use 'resource is None' as an indicator that this method is called in the main thread.
    If resource is not None, it is a threading.Lock() object which indicates that
    the method started in a separate thread. It has to be released
    before the snoop command is started.
    """
    testName = os.environ['PYTEST_CURRENT_TEST']
    logging.getLogger().info(f'{testName}, snoopToCsv: {csvFileName=}, {eventId=}, {mask=}, {duration=}, {resource=}')
    with open(csvFileName, 'wb') as file1:
      try:
        startTime = datetime.datetime.now()
        if not resource is None:
          print('snoopToCsv: Start Thread ', startTime.time())
          resource.release()
        process = subprocess.run(self.getSnoopCommand(eventId, mask, duration), shell=True, check=True, stdout=file1)
        endTime = datetime.datetime.now()
        if not resource is None:
          print(f'snoopToCsv: Return: {process.returncode:3d}   {endTime.time()}, duration: {endTime - startTime}')
        self.assertEqual(process.returncode, 0, f'Returncode: {process.returncode}')
      except Exception as exception:
        if resource is None:
          raise exception
        else:
          self.exc_info = sys.exc_info()

  def snoopToCsvWithAction(self, csvFileName, action, actionArgs=[], eventId='0', mask='0', duration=1):
    """Snoop timing messages with saft-ctl for <duration> seconds (default = 1).
    Write the messages to <csvFileName>.
    Details: start saft-ctl with Popen in its own thread, run it for <duration> seconds.
    action should end before snoop.
    """
    testName = os.environ['PYTEST_CURRENT_TEST']
    logging.getLogger().info(f'{testName}, snoopToCsvWithAction: {csvFileName=}, {eventId=}, {mask=}, {duration=}, {action=}, {actionArgs=}')
    self.exc_info = None
    print('snoopToCsv: acquire lock ', datetime.datetime.now().time())
    resource = threading.Lock()
    # wait at most 10 seconds for the thread to start.
    # Lock is released before the main action of the thread starts.
    resource.acquire(timeout=10.0)
    snoop = threading.Thread(target=self.snoopToCsv, args=(csvFileName, eventId, mask, duration, resource))
    snoop.start()
    resource.acquire(timeout=10.0)
    print('snoopToCsv: call action  ', datetime.datetime.now().time())
    try:
      if len(actionArgs) == 0:
        action()
      else:
        action(actionArgs)
    finally:
      print('snoopToCsv: finish action', datetime.datetime.now().time())
      snoop.join()
      resource.release()
      print('snoopToCsv: release lock ', datetime.datetime.now().time())
      if not self.exc_info is None:
        # ~ print(f'{self.exc_info=}')
        raise self.exc_info[1].with_traceback(self.exc_info[2])

  def analyseFrequencyFromCsv(self, csvFileName, column=20, printTable=True, checkValues=dict(), addDelayed=False):
    """Analyse the frequency of the values in the specified column. Default column is 20 (parameter of the timing message).
    Prints (if printTable=True) the table of values, counters, and frequency over the whole time span.
    Column for EVTNO is 8. Timing messages should have 'fid=1' otherwise column numbers are different.
    checkValues is a dictionary of key-value pairs to check. Key is a value in the column and value is the required frequency.
    The value can be '>n', '<n', '=n', 'n' (which is the same as '=n'), '=0'. The syntax '<n' fails if there are no occurrences.
    Checks for intervalls are not possible since checkValues is a dictionary and keys occur at most once.
    Example: column=8 and checkValues={'0x0001': 62} checks that EVTNO 0x0001 occurs in 62 lines of the file to analyse.
    Example: column=8 and checkValues={'0x0002': '>0'} checks that EVTNO 0x0002 occurs at least once in the file to analyse.
    Example: column=4 and checkValues={'0x7': '=0'} checks that FID 0x7 does NOT occur in the file to analyse.
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
        self.assertGreater(len(row), column, f'Column {column} does not exist. Maximal column {len(row)}. May be fid=1 should be used in schedule.')
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
          if "!delayed" in key or "!conflict" in key or "!late" in key:
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
          if "!late" in key:
            keyAligned = key + "    "
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
          if str(checkValues[item]) == '=0' or str(checkValues[item]) == '<1':
            self.assertFalse(item in listCounter.keys(), f'Key {item} found, should not occur')
          elif item in listCounter.keys():
            if str(checkValues[item])[0] == '>':
              if addDelayed and (item + '!delayed') in listCounter.keys():
                self.assertGreater(listCounter[item] + listCounter[item + '!delayed'], int(checkValues[item][1:]), f'assertGreater for {item}: is:{listCounter[item]} + { + listCounter[item + "!delayed"]} expected:{checkValues[item]}')
              else:
                self.assertGreater(listCounter[item], int(checkValues[item][1:]), f'assertGreater for {item}: is:{listCounter[item]} expected:{checkValues[item]}')
            elif str(checkValues[item])[0] == '<':
              if addDelayed and (item + '!delayed') in listCounter.keys():
                self.assertGreater(int(checkValues[item][1:]), listCounter[item] + listCounter[item + '!delayed'], f'assertSmaller for {item}: is:{listCounter[item]} + { + listCounter[item + "!delayed"]} expected:{checkValues[item]}')
              else:
                self.assertGreater(int(checkValues[item][1:]), listCounter[item], f'assertSmaller for {item}: is:{listCounter[item]} expected:{checkValues[item]}')
            elif str(checkValues[item])[0] == '=':
              if addDelayed and (item + '!delayed') in listCounter.keys():
                self.assertEqual(listCounter[item] + listCounter[item + '!delayed'], int(checkValues[item][1:]), f'assertEqual for {item}: is:{listCounter[item]} + { + listCounter[item + "!delayed"]} expected:{checkValues[item]}')
              else:
                self.assertEqual(listCounter[item], int(checkValues[item][1:]), f'assertEqual for {item}: is:{listCounter[item]} expected:{checkValues[item]}')
            else:
              if addDelayed and (item + '!delayed') in listCounter.keys():
                self.assertEqual(listCounter[item] + listCounter[item + '!delayed'], int(checkValues[item]), f'assertEqual for {item}: is:{listCounter[item]} + { + listCounter[item + "!delayed"]} expected:{checkValues[item]}')
              else:
                self.assertEqual(listCounter[item], int(checkValues[item]), f'assertEqual for {item}: is:{listCounter[item]} expected:{checkValues[item]}')
          elif addDelayed and item + '!delayed' in listCounter.keys():
            if str(checkValues[item])[0] == '>':
              self.assertGreater(listCounter[item + '!delayed'], int(checkValues[item][1:]), f'assertGreater for {item}: is:{listCounter[item + "!delayed"]} expected:{checkValues[item]}')
            elif str(checkValues[item])[0] == '<':
              self.assertGreater(int(checkValues[item][1:]), listCounter[item + '!delayed'], f'assertSmaller for {item}: is:{listCounter[item + "!delayed"]} expected:{checkValues[item]}')
            elif str(checkValues[item])[0] == '=':
              self.assertEqual(listCounter[item + '!delayed'], int(checkValues[item][1:]), f'assertEqual for {item}: is:{listCounter[item + "!delayed"]} expected:{checkValues[item]}')
            else:
              self.assertEqual(listCounter[item + '!delayed'], int(checkValues[item]), f'assertEqual for {item}: is:{listCounter[item + "!delayed"]} expected:{checkValues[item]}')
          else:
            self.assertTrue(item in listCounter.keys(), f'Key {item} not found, but expected.')

  def analyseDmCmdOutput(self, threadsToCheck='', useVerbose=False) -> dict:
    """Collect the message counts for all threads. Use dm-cmd to get the
    status with the message counts. Return a dict with the message counts.
    Key: first letter is cpu, next two letters thread number.
    Example: key=123 is cpu 1, thread 23.
    Assumption: less than 10 cpus, less than 100 threads.
    """
    if useVerbose:
      outputStdoutStderr = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-v'), [0])
      offsetLines = 7 + 16 + self.cpuQuantity
    else:
      outputStdoutStderr = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster), [0])
      offsetLines = 7 + self.cpuQuantity
    output = outputStdoutStderr[0]
    testName = os.environ['PYTEST_CURRENT_TEST']
    logging.getLogger().debug(f'{testName}, analyseDmCmdOutput' + '\n' + '\n'.join(output))
    msgCounts = {}
    threadsCheck = '0' * (self.threadQuantity * self.cpuQuantity)
    index = 0
    countLines = len(output)
    offset = 0
    offset1 = 0
    cpu = 0
    thread = 0
    running = ''
    for line in output:
      if index > offsetLines and index < (countLines - 1):
        try:
          cpu = int(line[3 + offset1])
          if line[8 + offset1] == ' ':
            thread = int(line[9 + offset1])
          else:
            thread = int(line[8 + offset1:10 + offset1])
          running = line[21 + offset1:24 + offset1]
          count = int(line[32 + offset:42 + offset])
          msgCounts[f'{cpu:1d}{thread:02d}'] = str(count)
          if running == 'yes':
            threadsCheck = threadsCheck[:self.threadQuantity * cpu + thread] + '1' + threadsCheck[self.threadQuantity * cpu + thread + 1:]
          # ~ print(f'threadsCheck: {threadsCheck}, threadsToCheck: {threadsToCheck}, running:{running} {cpu} {thread} "{line[8 + offset1:10 + offset1]}"')
        except ValueError:
          print(f'ValueError:{index} CPU {cpu} Thread {thread}  line: "{line}", "{line[3 + offset1]}", "{line[9 + offset1]}", "{line[32 + offset:42 + offset]}", offset={offset}, offset1={offset1}')
        if offset == 0:
          offset = 10
          offset1 = 5
        else:
          offset = 0
          offset1 = 0
      index = index + 1
    self.assertEqual(countLines - offsetLines - 2, len(msgCounts), f'Output has {countLines} lines, messages: {len(msgCounts)}')
    if threadsToCheck != '':
      self.assertEqual(threadsCheck, threadsToCheck, f'threads running: {threadsCheck}, expected: {threadsToCheck}')
    return msgCounts

  def checkRunningThreadsCmd(self, messageInterval=1.0, cpuList=[0,1,2,3]):
    """Check that threads are running by comparison of message counts.
    Assumption: there is at least one timing message for this thread in
    messageIntervall (default 1.0 second).
    dm-cmd runs twice and the message count for the second run must be
    greater than the first message count. In addition the first
    message count should be grater than 0.
    """
    firstCounts = self.analyseDmCmdOutput()
    self.delay(messageInterval)
    secondCounts = self.analyseDmCmdOutput()
    for key in firstCounts:
      firstCount = int(firstCounts[key])
      secondCount = int(secondCounts[key])
      cpu = int(key[0])
      if cpu in cpuList:
        thread = int(key[1:])
        # ~ print(f'{key=}, {firstCount=}, {secondCount=}, {key[0]=}, {cpu=}, {key[1:]=}, {thread=}')
        self.assertGreater(secondCount, firstCount, f'CPU {cpu} Thread {thread} First: {firstCount}, second: {secondCount}')
        self.assertGreater(firstCount, 0, f'CPU {cpu} Thread {thread} firstCount is {firstCount}')

  def getQuantity(self, line):
    """Get the quantity of command executions from the output line of 'dm-cmd <datamaster> queue -v <block name>'.
    Search for 'Qty: ' in the line and parse the number.
    """
    pos = line.find('Qty: ')
    pos1 = line.find(' ', pos + len('Qty: '))
    quantity = 0
    if pos > -1 and pos1 > pos:
      quantity = int(line[pos + len('Qty: '):pos1])
    return quantity

  def checkQueueFlushed(self, queuesToFlush, blockName, flushPrio, checkFlush=True):
    """ Check that a flush command is executed and the defined queues are flushed.
    queuesToFlush: binary value between 0 and 7 for the queues to flush.
    blockName: name of the block with the queues to check.
    flushPrio: priority of the flush command. Defines the queue where to look for the flush command.
    Priority queues higher than the flushPrio are not affected by the flush command since these
    queues are empty when a flush command with lower priority is executed. Therefore queuesToFlush is changed for
    flushprio = 0, 1.
    The check for the executed flush command and the checks for the flushed queues are independant. All checks
    are done in one parse run of the output of 'dm-cmd <datamaster> -v queue <blockName>'. Flags and counters
    are used to signal the section inside the output.
    checkFlush = False means: check that no flush is executed.
    """
    output = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-v', 'queue', blockName), [0], 38)
    checkQueue0 = False
    check0 = False
    checkQueue1 = False
    check1 = False
    checkQueue2 = False
    check2 = False
    checkFlush0 = False
    checkFlush1 = False
    checkFlush2 = False
    flushExecuted = False
    for line in output:
      # check that flush is executed (there is exactly 1 flush executed)
      if checkFlush0:
        # ~ print(f'0 {counterFlush0} {flushExecuted} flushPrio {flushPrio}, {"empty" in line}, {"CmdType: flush" in line}, qty: {self.getQuantity(line)}')
        flushExecuted = flushExecuted or (flushPrio == 0 and 'empty' in line and 'CmdType: flush' in line and self.getQuantity(line) == 0)
        counterFlush0 = counterFlush0 + 1
        if counterFlush0 > 3:
          checkFlush0 = False
      if flushPrio == 0 and 'Priority 0 (' in line:
        checkFlush0 = True
        counterFlush0 = 0
      if checkFlush1:
        # ~ print(f'1 {counterFlush1} {flushExecuted} flushPrio {flushPrio}, {"empty" in line}, {"CmdType: flush" in line}, qty: {self.getQuantity(line)}')
        flushExecuted = flushExecuted or (flushPrio == 1 and 'empty' in line and 'CmdType: flush' in line and self.getQuantity(line) == 0)
        counterFlush1 = counterFlush1 + 1
        if counterFlush1 > 3:
          checkFlush1 = False
      if flushPrio == 1 and 'Priority 1 (' in line:
        checkFlush1 = True
        counterFlush1 = 0
      if checkFlush2:
        # ~ print(f'2 {counterFlush2} {flushExecuted} flushPrio {flushPrio}, {"empty" in line}, {"CmdType: flush" in line}, qty: {self.getQuantity(line)}')
        flushExecuted = flushExecuted or (flushPrio == 2 and 'empty' in line and 'CmdType: flush' in line and self.getQuantity(line) == 0)
        counterFlush2 = counterFlush2 + 1
        if counterFlush2 > 3:
          checkFlush2 = False
      if flushPrio == 2 and 'Priority 2 (' in line:
        checkFlush2 = True
        counterFlush2 = 0

      # check that queues are flushed (0 to 3 queues can be flushed)
      # ~ print(f"{checkQueue0}, {check0}, {'empty' in line}, {self.getQuantity(line)}, {line}")
      if checkQueue0:
        check0 = ('empty' in line and self.getQuantity(line) > 0)
        counter0 = counter0 + 1
        if counter0 > 3:
          checkQueue0 = False
      if not checkQueue0 and queuesToFlush & 0x1 and 'Priority 0 (' in line:
        # start check of queue 0
        checkQueue0 = True
        counter0 = 0
      if checkQueue1:
        check1 = ('empty' in line and self.getQuantity(line) > 0)
        counter1 = counter1 + 1
        if counter1 > 3:
          checkQueue1 = False
      if not checkQueue1 and queuesToFlush & 0x2 and 'Priority 1 (' in line:
        # start check of queue 1
        checkQueue1 = True
        counter1 = 0
      if checkQueue2:
        check2 = ('empty' in line and self.getQuantity(line) > 0)
        counter2 = counter2 + 1
        if counter2 > 3:
          checkQueue2 = False
      if not checkQueue2 and queuesToFlush & 0x4 and 'Priority 2 (' in line:
        # start check of queue 2
        checkQueue2 = True
        counter2 = 0
    if flushPrio == 0 and queuesToFlush > 1:
      queuesToFlush = queuesToFlush & 0x1
    if flushPrio == 1 and queuesToFlush > 1:
      queuesToFlush = queuesToFlush & 0x3
    queuesFlushed = 0
    if queuesToFlush & 0x1 and check0:
      queuesFlushed = queuesFlushed + 1
    if queuesToFlush & 0x2 and check1:
      queuesFlushed = queuesFlushed + 2
    if queuesToFlush & 0x4 and check2:
      queuesFlushed = queuesFlushed + 4
    self.assertEqual(queuesFlushed, queuesToFlush, f'Queue 2 flushed {check2}, Queue 1 flushed {check1}, Queue 0 flushed {check0}, Queues to check: {queuesToFlush:03b}')
    if checkFlush:
      self.assertTrue(flushExecuted, f'flushExecuted: {flushExecuted}, queuesToFlush: {queuesToFlush}')
    else:
      self.assertFalse(flushExecuted, f'flushExecuted: {flushExecuted}, queuesToFlush: {queuesToFlush}')

  def inspectQueue(self, node, read=0, write=0, pending=0, retry=False):
    # check the result with dm-cmd ... queue <node>.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'queue', node), [0], 10, 0)
    print('1. try', '\n'.join(lines))
    try:
      self.verifyInspectingQueueOutput(lines, node, read, write, pending)
    except AssertionError as instance:
      testName = os.environ['PYTEST_CURRENT_TEST']
      logging.getLogger().warning(f'{testName}, AssertionError while Inspecting Queues, try second inspection.' + '\n' + '\n'.join(lines))
      self.assertEqual(instance.args[0][:111], f"'Priority 0 (priolo)  RdIdx: 0 WrIdx: 1    Pending: 1' != 'Priority 0 (priolo)  RdIdx: {read} WrIdx: {write}    Pending: {pending}", 'current exception is ' + instance.args[0])
      # second try: check the result with dm-cmd ... queue <node>.
      if retry:
        self.delay(1.0)
        lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'queue', node), [0], 10, 0)
        print('2. try', '\n'.join(lines))
        self.verifyInspectingQueueOutput(lines, node, 1, 1, 0)

  def verifyInspectingQueueOutput(self, lines, node, read, write, pending):
    # verify the output (lines 1 to 9).
    expectedLines = ['', f'Inspecting Queues of Block {node}',
    'Priority 2 (prioil)  Not instantiated',
    'Priority 1 (priohi)  Not instantiated',
    f'Priority 0 (priolo)  RdIdx: {read} WrIdx: {write}    Pending: {pending}',
    '', '', '', '']
    for i in range(0,4):
      expectedLines[i+5] = f'#{(i+read) % 4} empty   -'
    if pending == 1:
      expectedLines[5] = '#0 pending Valid Time: 0x0000000000000000 0000000000000000000    CmdType: noop    Qty: 1'
    for i in range(1,9):
      self.assertEqual(lines[i], expectedLines[i], 'wrong output, expected: ' + expectedLines[i] + '\n' + '\n'.join(lines))

  def delay(self, duration):
    """Delay for <duration> seconds. <duration> is a float.
    """
    time.sleep(duration)

  def runThreadXCommand(self, cpu, thread, command):
    """Test for one CPU and one thread with 'command'.
    Check the return code of 0 for success and one line of output on stdout.
    """
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', command), [0], 1, 0)

  def prepareRunThreads(self, cpus=15):
    """Check that no thread runs on the CPUs given by 'cpu' (bit mask).
    Load schedules into DM, one for each CPU and start all threads.
    Check that these are running.
    """
    testName = os.environ['PYTEST_CURRENT_TEST']
    logging.getLogger().debug(f'{testName} prepare threads {datetime.datetime.now()}')
    cpuList = self.listFromBits(cpus, self.cpuQuantity)
    cpuMask = self.maskFromList(cpuList, self.cpuQuantity)
    # Add schedules for all CPUs and start pattern on all threads.
    for cpu in cpuList:
      self.addSchedule(f'pps-all-threads-cpu{cpu}.dot')
    # Check all CPUs that no thread is running.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', cpuMask, 'running'), [0], len(cpuList), 0)
    for i in range(len(cpuList)):
      expectedText = 'CPU {variable} Running Threads: 0x0'.format(variable=i)
      try:
        self.assertEqual(lines[i], expectedText, 'wrong output, expected: ' + expectedText + '\n' + '\n'.join(lines))
      except AssertionError as exception:
        # second try to check the running threads
        # dm-cmd -c cpuMask running shows the same result as the first call. So halt all threads before.
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'halt'), [0], 0, 0)
        lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', cpuMask, 'running'), [0], len(cpuList), 0)
        for i in range(len(cpuList)):
          expectedText = 'CPU {variable} Running Threads: 0x0'.format(variable=i)
          self.assertEqual(lines[i], expectedText, 'wrong output, expected: ' + expectedText + '\n' + '\n'.join(lines))
    if self.threadQuantity == 32:
      threadMask = '0xffffffff'
      threadList = [('a', '0x01010101'), ('b', '0x02020202'), ('c', '0x04040404'), ('d', '0x08080808'), ('e', '0x10101010'), ('f', '0x20202020'), ('g', '0x40404040'), ('h', '0x80808080')]
    elif self.threadQuantity == 8:
      threadMask = '0xff'
      threadList = [('a', '0x01'), ('b', '0x02'), ('c', '0x04'), ('d', '0x08'), ('e', '0x10'), ('f', '0x20'), ('g', '0x40'), ('h', '0x80')]
    else:
      self.assertFalse(True, f'threadQuantity is {self.threadQuantity}, allowed: 8 or 32')
    # Start pattern for all CPUs in cpuList and all threads
    for x, thread in threadList:
      for cpu in cpuList:
        patternName = f'PPS{cpu}' + x
        logging.getLogger().debug(f'{testName} {patternName} {cpu} {thread} {datetime.datetime.now()}')
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', patternName, '-t', thread), [0])
    self.checkRunningThreadsCmd(cpuList=cpuList)
    # Check all CPUs that all threads are running.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', cpuMask, 'running'), [0], len(cpuList), 0)
    for i in range(len(cpuList)):
      if i in cpuList:
        expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
      else:
        expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask='0x0')
      try:
        self.assertEqual(lines[i], expectedText, 'wrong output, expected: ' + expectedText + '\n' + '\n'.join(lines))
      except AssertionError as exception:
        self.delay(1.0)
        lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', cpuMask, 'running'), [0], len(cpuList), 0)
        for i in range(len(cpuList)):
          if i in cpuList:
            expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
          else:
            expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask='0x0')
          self.assertEqual(lines[i], expectedText, 'wrong output, expected: ' + expectedText + '\n' + '\n'.join(lines))
    logging.getLogger().debug(f'{testName}         threads {datetime.datetime.now()}')

  def deleteFile(self, fileName):
    """Delete file <fileName>.
    """
    if self.deleteFiles.lower() == 'true':
      fileToRemove = pathlib.Path(fileName)
      if fileToRemove.exists():
        fileToRemove.unlink()

  def resetAllCpus(self):
    """Reset each CPU (loop over all lm32 CPUs).
    """
    for cpu in range(self.cpuQuantity):
      self.startAndCheckSubprocess((self.getEbResetCommand(), self.datamaster, 'cpureset', str(cpu)), [0])
    self.delay(2.0)

  def maskFromList(self, itemList, quantity) -> str:
    mask = 0
    for item in itemList:
      if isinstance(item, int) and item < quantity:
        mask = (1 << item) | mask
    return f'0x{mask:0x}'

  def listFromBits(self, bits, quantity) -> list:
    """Convert a 'bits', given as a string or an int, into a list of
    int items. quantity is the maximal int + 1.
    """
    itemList = []
    if isinstance(bits, str):
      bits = int(bits, base=16)
    for i in range(quantity):
      if (1 << i) & bits > 0:
        itemList.append(i)
      # ~ print(f'{bits=}, {i=}, {itemList=}, {((1 << i) & bits)=}')
    return itemList

  def bitCount(self, bits, quantity) -> int:
    """Count how many bits are 1 in the number 'bits'.
    This is the number of items enabled in 'bits'.
    """
    if isinstance(bits, str):
      bits = int(bits, base=16)
    count = 0
    for i in range(quantity):
      # ~ print(f'{i=}, {(1 << i)=}, {bits=}, {count=}')
      if (1 << i) & bits > 0:
        count = count + 1
    return count

  def printStdOutStdErr(self, lines):
    """Print the lines of stdout and stderr. This is given as a list of
    two lists of lines.
    """
    if len(lines[0]) > 0:
      print(f'{chr(10).join(lines[0])}')
    if len(lines[1]) > 0:
      print(f'{chr(10).join(lines[1])}')

  @classmethod
  def getThreadQuantityFromFirmware(self) -> [int, int]:
    """This class method uses 'eb-info -w' to get the thread quantity
    and the CPU quantity from the lm32 firmware.
    This method is used once in the set up of a class by setUpClass.
    """
    threads = 8
    cpus = 4
    # pass cmd and args to the function
    process = subprocess.Popen(('eb-info', '-w', self.datamaster), stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    self.assertTrue(process.returncode in [0], f'wrong return code {process.returncode}, expected: [0], '
          + f'stderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    lines = stdout.decode("utf-8").splitlines()
    for line in lines:
      if 'ThreadQty   : 32' in line:
        threads = 32
      if 'holding a Firmware ID' in line:
        cpus = int(line[14])
    return cpus, threads

  @classmethod
  def logToFile(self, text, fileName):
    """This class method logs a text to a fileName. The text is prefixed
    by the current test name. This is appended to the file.
    """
    testName = os.environ['PYTEST_CURRENT_TEST']
    with open(fileName, 'a') as file1:
      file1.write(testName + ': ' + text + '\n')

  def checkRunningThreads(self, lines, masks) -> str:
    """Check the hex numbers describing the running threads.
    Since in some cases it is not determined which thread is used for a
    command, we have to check the lines against multiple masks.
    :param lines is a list of lines, describes the running threads.
    :param masks is a list (not a set!) of hex numbers as strings.
    Each number describes an allowed pattern of running threads.
    """
    remainingMasks = masks.copy()
    threadState = ''
    for line in lines:
      pos = line.find('0x')
      if (pos != -1):
        threadState = line[pos:]
        self.assertTrue(threadState in remainingMasks, f'Wrong running state. {threadState} not in {remainingMasks}, line {lines.index(line)}.')
        remainingMasks.remove(threadState)
    # ~ self.assertEqual(0, len(remainingMasks),f'Remaining masks: {remainingMasks}')
    return threadState

  def printTimestamp(self, text):
    """Print the current timestamp with some preceeding text.
    """
    print(text, datetime.datetime.now().time())
