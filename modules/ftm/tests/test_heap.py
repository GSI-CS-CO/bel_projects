import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'heap'.
"""
class HeapTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testInspectHeap(self):
    self.runInspectHeapThreads()

  @pytest.mark.thread32
  def testInspectHeap32(self):
    self.threadQuantity = 32
    self.runInspectHeapThreads()

  def runInspectHeapThreads(self):
    """Load a schedule and start all threads. Check that these are running.
    Inspect the heap of some threads. Check that these are not running.
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
    # Inspect heap for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'heap'), [0], (self.threadQuantity + 1) * cpuCount, 0)
    # ~ self.printStdOutStdErr(lines)

  @pytest.mark.thread8
  def testAbortSingleThreadDecimal(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread, 'heap')

  @pytest.mark.thread8
  def testAbortSingleThreadHex(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}', 'heap')

  @pytest.mark.thread32
  def testAbortSingleThreadDecimal(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    self.threadQuantity = 32
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread, 'heap')

  @pytest.mark.thread32
  def testAbortSingleThreadHex(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    self.threadQuantity = 32
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}', 'heap')

  def runThreadXCommand(self, cpu, thread, command, assertText=''):
    """Test for one thread. If commandSet=True set the time (parameter) with the command.
    In all cases, read this value. Check the output of both commands.
    """
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', command), [0], self.threadQuantity + 1, 0)
