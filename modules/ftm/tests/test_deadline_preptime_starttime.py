import subprocess
import pytest          # @pytest.mark.slow
import dm_testbench    # contains super class
import math            # contains log2()

"""
Module collects tests for dm-cmd with the commands preptime, starttime, deadline.
"""
class ThreadBitsTest(dm_testbench.DmTestbench):

  threadQuantity = 8
  threadMask = (1 << threadQuantity) - 1

  def threadCount(self, thread):
    """Count how many bits are 1 in the number 'thread'.
    This is the number of threads enabled in the number.
    """
    threadCount = 0
    for i in range(self.threadQuantity):
      print(f'{i=}, {(1 << i)=}, {thread=}, {threadCount=}')
      if (1 << i) & thread > 0:
        threadCount = threadCount + 1
    return threadCount

  def testPreptimeSingleThreadDecimal(self):
    """Loop for all threads setting and reading the preptime.
    Uses the thread number in decimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(thread, 'preptime', '100000000')

  def testPreptimeSingleThreadHex(self):
    """Loop for all threads setting and reading the preptime.
    Uses the thread number in hexadecimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(f'0x{(1 << thread):x}', 'preptime', '100000000')

  def testStarttimeSingleThreadDecimal(self):
    """Loop for all threads setting and reading the starttime.
    Uses the thread number in decimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(thread, 'starttime', '100000000')

  def testStarttimeSingleThreadHex(self):
    """Loop for all threads setting and reading the starttime.
    Uses the thread number in hexadecimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(f'0x{(1 << thread):x}', 'starttime', '100000000')

  def testDeadlineSingleThreadDecimal(self):
    """Loop for all threads reading the deadline.
    Uses the thread number in decimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(thread, 'deadline', '100000000', commandSet=False)

  def testDeadlineSingleThreadHex(self):
    """Loop for all threads reading the deadline.
    Uses the thread number in hexadecimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(f'0x{(1 << thread):x}', 'deadline', '100000000', commandSet=False)

  def testCursorSingleThreadDecimal(self):
    """Loop for all threads reading the cursor.
    Uses the thread number in decimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(thread, 'cursor', '0', commandSet=False, assertText='Currently at idle')

  def testCursorSingleThreadHex(self):
    """Loop for all threads reading the cursor.
    Uses the thread number in hexadecimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(f'0x{(1 << thread):x}', 'cursor', '0', commandSet=False, assertText='Currently at idle')

  def testOriginSingleThreadDecimal(self):
    """Loop for all threads reading the origin.
    Uses the thread number in decimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(thread, 'origin', '0', commandSet=False, assertText='CPU 0 Thread {variable} origin points to node idle')

  def testOriginSingleThreadHex(self):
    """Loop for all threads reading the origin.
    Uses the thread number in hexadecimal form.
    """
    for thread in range(self.threadQuantity):
      self.runThreadXCommand(f'0x{(1 << thread):x}', 'origin', '0', commandSet=False, assertText='CPU 0 Thread {variable} origin points to node idle')

  def runThreadXCommand(self, thread, command, parameter, commandSet=True, assertText=''):
    """Test for one thread. If commandSet=True set the time (parameter) with the command.
    In all cases, read this value. Check the output of both commands.
    """
    threads = []
    if isinstance(thread, str):
      threadD = self.threadQuantity
      for i in range(self.threadQuantity):
        if f'0x{(1 << i):x}' == thread:
          threadD = i
          threads.append(i)
          break
      threadCount = self.threadCount(2**threadD)
    else:
      threadD = thread
      threads.append(thread)
      threadCount = self.threadCount(2**thread)
    # ~ print(f'{thread=}, {threadD=}, {type(thread)=}, {threadCount=}, {threads=}')
    if commandSet:
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', command, parameter), [0], threadCount, 0)
      for i in range(threadCount):
        self.assertEqual(lines[i], f'setting {command}: CPU 0 Thread {threads[i]}.', 'wrong output')
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', command), [0], threadCount, 0)
    for i in range(threadCount):
      positionDeadline = lines[i].find('Deadline')
      if assertText == '':
        text = f'CPU 0 Thr {threads[i]} {command.capitalize()} {parameter}'
      else:
        text = assertText.format(variable=threads[i])
      if positionDeadline == -1:
        self.assertEqual(lines[i], text, 'wrong output')
      else:
        self.assertEqual(lines[i][0:positionDeadline], f'CPU 0 Thr {threads[i]} ', 'wrong output')

  def testThreadHex(self):
    """Test for two threads. First set the time (parameter) with the command, then read this value. Check the output of both commands.
    """
    thread = '0x3'
    command = 'preptime'
    parameter = '10000000'
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', command, parameter), [0], 2, 0)
    print(f'{lines=}')
    self.assertEqual(lines[0], f'setting preptime: CPU 0 Thread 0.', 'wrong output')
    self.assertEqual(lines[1], f'setting preptime: CPU 0 Thread 1.', 'wrong output')
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', command), [0], 2, 0)
    print(f'{lines=}')
    self.assertEqual(lines[0], f'CPU 0 Thr 0 {command.capitalize()} {parameter}', 'wrong output')
    self.assertEqual(lines[1], f'CPU 0 Thr 1 {command.capitalize()} {parameter}', 'wrong output')

  @pytest.mark.thread8
  def testThreadMaxPlusOne(self):
    """Set the thread number 'threadQuantity + 1'. Should return code 255.
    Only useful for 8 threads.
    """
    thread = f'0x{(1 << self.threadQuantity):x}'
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread mask '{thread}' is invalid. Choose a mask that fits to 0x{self.threadMask:x}.", 'wrong output')

  @pytest.mark.thread32
  def testThreadMaxPlusOne32(self):
    """Set the thread number 'threadQuantity + 1'. Should return code 255.
    Only useful for 32 threads.
    """
    thread = f'0x{(1 << 32):x}'
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread mask '{thread}' is invalid. Choose a mask that fits to 0x{self.threadMask:x}.", 'wrong output')

  @pytest.mark.slow
  def testThread256(self):
    """Test all combinations of threads from 0 to 255 (0xFF).
    """
    for thread in range(256):
      threadCount = self.threadCount(thread)
      threadX = f'0x{thread:x}'
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [0], threadCount, 0)
      # check stdout for thread 0 if thread 0 is involved (last bit is set in 'thread').
      if thread & 1 > 0:
        self.assertEqual(lines[0], f'CPU 0 Thr 0 Preptime 1000000', 'wrong output {threadX} {thread}')

  @pytest.mark.slow
  def testThreadFullRange(self):
    """Test all combinations of threads from 0 to 255 (0xFF) or to 0xFFF.
    Depends on threadQuantity.
    """
    threads = min(self.threadQuantity, 12)
    for thread in range(2**threads):
      threadCount = self.threadCount(thread)
      threadX = f'0x{thread:x}'
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [0], threadCount, 0)
      # check stdout for thread 0 if thread 0 is involved (last bit is set in 'thread').
      if thread & 1 > 0:
        self.assertEqual(lines[0], f'CPU 0 Thr 0 Preptime 1000000', 'wrong output {threadX} {thread}')

  @pytest.mark.thread8
  def testThread0x100(self):
    """Set a bit in the thread mask outside of range. Should return code 255.
    """
    threadX = '0x100'
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread mask '0x100' is invalid. Choose a mask that fits to 0xff.", 'wrong output')

  @pytest.mark.thread8
  def testOutOfRange(self):
    """Test thread 8. If threadQuantity = 8, this is out of range.
    """
    threadX = '8'
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread idx '8' is invalid. Choose an index between 0 and 7.", 'wrong output')

  def testMissingArgument(self):
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread argument 'preptime' is invalid. Not a number.", 'wrong output')

  def testMissingArgumentVerbose(self):
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-v', '-t', 'preptime'), [255], 2, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread argument 'preptime' is invalid. Not a number.", 'wrong output')

  def tearDown(self):
    # reset CPU 0 to get a clean state. This is not done by dm-cmd reset all.
    self.resetAllCpus()

