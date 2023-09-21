import subprocess
import pytest          # @pytest.mark.slow
import dm_testbench    # contains super class
import math            # contains log2()

"""
Module collects tests for threadBits
"""
class ThreadBitsTest(dm_testbench.DmTestbench):

  threadQuantity = 8
  threadMask = (1 << threadQuantity) - 1

  def testThread0(self):
    self.runThreadX(0)

  def testThread1(self):
    self.runThreadX(1)

  def testThread2(self):
    self.runThreadX(2)

  def testThread3(self):
    self.runThreadX(3)

  def testThread4(self):
    self.runThreadX(4)

  def testThread5(self):
    self.runThreadX(5)

  def testThread6(self):
    self.runThreadX(6)

  def testThread7(self):
    self.runThreadX(7)

  def runThreadX(self, thread):
    self.runThreadXCommand(thread, 'preptime', '100000000')

  def runThreadXCommand(self, thread, command, parameter):
    """Test for one thread. First set the time (parameter) with the command, then read this value. Check the output of both commands.
    """
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', command, parameter), [0], 1, 0)
    print(f'{lines=}')
    self.assertEqual(lines[0], f'setting {command}: CPU 0 Thread {thread:d}.', 'wrong output')
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', command), [0], 1, 0)
    print(f'{lines=}')
    self.assertEqual(lines[0], f'CPU 0 Thr {thread:d} {command.capitalize()} {parameter}', 'wrong output')

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

  def testThreadMaxPlusOne(self):
    """
    """
    thread = f'0x{(1 << self.threadQuantity):x}'
    if self.threadQuantity > 31 and int(thread, base=16) & self.threadMask == 0:
      lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', '-v', 'preptime'), [0], 1, 0)
      print(f'{thread=} {lines=}')
      self.assertEqual(lines[0][0], f"{self.binaryDmCmd}: verbose: 1 thread: 0x0 cpu: 0 error: 0", 'wrong output')
    else:
      lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', f'{thread}', '-v', 'preptime'), [255], 0, 1)
      print(f'{thread=} {lines=}')
      self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread mask '{thread}' is invalid. Choose a mask that fits to 0x{self.threadMask:x}.", 'wrong output')

  @pytest.mark.slow
  def testThread256(self):
    """
    """
    for thread in range(256):
      threadCount = 0
      for i in range(8):
        if (1 << i) & thread > 0:
          threadCount = threadCount + 1
      threadX = f'0x{thread:x}'
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [0], threadCount, 0)
      # check stdout for thread 0 if thread 0 is involved (last bit is set in 'thread'.
      if thread & 1 > 0:
        self.assertEqual(lines[0], f'CPU 0 Thr 0 Preptime 1000000', 'wrong output {threadX} {thread}')

  @pytest.mark.slow
  def testThreadFullRange(self):
    """
    """
    threads = min(self.threadQuantity, 12)
    for thread in range(2**threads):
      threadCount = 0
      for i in range(threads):
        if (1 << i) & thread > 0:
          threadCount = threadCount + 1
      threadX = f'0x{thread:x}'
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [0], threadCount, 0)
      # check stdout for thread 0 if thread 0 is involved (last bit is set in 'thread'.
      if thread & 1 > 0:
        self.assertEqual(lines[0], f'CPU 0 Thr 0 Preptime 1000000', 'wrong output {threadX} {thread}')

  def testSingleThread(self):
    """Loop for all threads reading the preptime.
    """
    for thread in range(self.threadQuantity):
      threadCount = 1
      threadX = f'0x{(1 << thread):x}'
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', threadX, 'preptime'), [0], threadCount, 0)
      self.assertEqual(lines[0], f'CPU 0 Thr {thread} Preptime 1000000', 'wrong output')

  def testThread0x100(self):
    """
    """
    threadX = '0x100'
    if math.log2(int(threadX, base=16)) in range(self.threadQuantity):
      lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', threadX, '-v', 'preptime'), [0], 2, 0)
      self.assertEqual(lines[0][0], f"{self.binaryDmCmd}: verbose: 1 thread: {threadX} cpu: 0 error: 0", 'wrong output')
    else:
      lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', threadX, '-v', 'preptime'), [255], 0, 1)
      self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread mask '0x100' is invalid. Choose a mask that fits to 0xff.", 'wrong output')

  def testOutOfRange(self):
    """Test thread 8. If threadQuantity = 8, this is out of range.
    Otherwise, threadBits should return 0 and the correct output.
    """
    threadX = '8'
    if int(threadX) in range(self.threadQuantity):
      lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', threadX, '-v', 'preptime'), [0], 2, 0)
      self.assertEqual(lines[0][0], f"{self.binaryDmCmd}: verbose: 1 thread: 0x{(1 << int(threadX)):x} cpu: 0 error: 0", 'wrong output')
    else:
      lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', threadX, '-v', 'preptime'), [255], 0, 1)
      self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread idx '8' is invalid. Choose an index between 0 and 7.", 'wrong output')

  def testMissingArgument(self):
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-t', 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread argument 'preptime' is invalid. Not a number.", 'wrong output')

  def testMissingArgumentVerbose(self):
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-v', '-t', 'preptime'), [255], 0, 1)
    self.assertEqual(lines[1][0], f"{self.binaryDmCmd}: Thread argument 'preptime' is invalid. Not a number.", 'wrong output')

  def tearDown(self):
    # reset preptime back to 1000000 after test. This is not done by dm-cmd reset all.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'preptime', '-c0', '-t0xff', '1000000'), linesCout=8, linesCerr=0)
