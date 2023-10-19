import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'abort'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class AbortTests(dm_testbench.DmTestbench):

  def setUp(self):
    """Setup CPU quantity and thread quantity for 8 threads.
    Tests for 32 threads must change the threadQuantity before any test
    action.
    """
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testAbortRunningThreads(self):
    self.runAbortRunningThreads()

  @pytest.mark.thread32
  def testAbortRunningThreads32(self):
    self.threadQuantitiy = 32
    self.runAbortRunningThreads()

  def runAbortRunningThreads(self):
    """Check that no thread runs on 4 CPUs. Load 4 schedules, one for
    each CPU and start all threads. Check that these are running.
    Abort some threads. Check that these are not running.
    """
    # Check all CPUs that no thread is running.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
    # ~ self.printStdOutStdErr(lines)
    for i in range(self.cpuQuantity):
      self.assertEqual(lines[0][i], f'CPU {i} Running Threads: 0x0', 'wrong output')
    # Add schedules for all CPUs and start pattern on all threads.
    self.addSchedule('pps-all-threads-cpu0.dot')
    self.addSchedule('pps-all-threads-cpu1.dot')
    self.addSchedule('pps-all-threads-cpu2.dot')
    self.addSchedule('pps-all-threads-cpu3.dot')
    index = 0
    threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3'), ('e', '4'), ('f', '5'), ('g', '6'), ('h', '7')]
    for x, y in threadList:
      if index < self.threadQuantity:
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS0' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS1' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS2' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS3' + x, '-t', y), [0])
        index = index + 1
    self.checkRunningThreadsCmd()
    # Check all CPUs that all threads are running.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
    # ~ self.printStdOutStdErr(lines)
    for i in range(self.cpuQuantity):
      if self.threadQuantity > 8:
        self.assertEqual(lines[0][i], f'CPU {i} Running Threads: 0xffffffff', 'wrong output')
      else:
        self.assertEqual(lines[0][i], f'CPU {i} Running Threads: 0xff', 'wrong output')
    # Abort some threads on CPUs 0 and 1
    cpu = '0x3' # CPUs 0 and 1
    thread = '0xaa' # Threads 2, 4, 6, 8
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'abort'), [0], threadCount * cpuCount, 0)
    # ~ self.printStdOutStdErr(lines)
    threads = self.listFromBits(thread, self.threadQuantity)
    cpus = self.listFromBits(cpu, self.cpuQuantity)
    for i in range(cpuCount):
      for j in range(threadCount):
        self.assertEqual(lines[0][i*threadCount+j], f'CPU {cpus[i]} Thread {threads[j]} aborted.', 'wrong output')
    # Check that the remaining threads are running
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], 4, 0)
    # ~ self.printStdOutStdErr(lines)
    # define the thread masks for 32 and 8 threads.
    if self.threadQuantity == 32:
      threadMask = '0xffffffff'
      threadMaskAborted = '0xffffff55'
    elif self.threadQuantity == 8:
      threadMask = '0xff'
      threadMaskAborted = '0x55'
    else:
      self.assertFalse(True, f'threadQuantity is {self.threadQuantity}, allowed: 8 or 32')
    # compare the lines of stdout with expected texts.
    for i in range(self.cpuQuantity):
      if i in cpus:
        expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMaskAborted)
        messageText = 'wrong output, expected: CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMaskAborted)
        self.assertEqual(lines[0][i], expectedText, messageText)
      else:
        expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
        messageText = 'wrong output, expected: CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
        self.assertEqual(lines[0][i], expectedText, messageText)

  @pytest.mark.thread8
  def testAbortSingleThreadDecimal(self):
    """Loop over all CPUs and all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread, 'abort')

  @pytest.mark.thread8
  def testAbortSingleThreadHex(self):
    """Loop over all CPUs and all threads aborting this thread.
    Uses the thread number in hexadecimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}', 'abort')

  @pytest.mark.thread32
  def testAbortSingleThreadDecimal32(self):
    """Loop over all CPUs and all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    self.threadQuantity = 32
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread, 'abort')

  @pytest.mark.thread32
  def testAbortSingleThreadHex32(self):
    """Loop over all CPUs and all threads aborting this thread.
    Uses the thread number in hexadecimal form.
    """
    self.threadQuantity = 32
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}', 'abort')

  def tearDown(self):
    super().tearDown()
    # reset all CPUs to get a clean state. This is not done by dm-cmd reset all.
    self.resetAllCpus()
