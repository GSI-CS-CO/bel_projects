import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'heap'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
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
    """Prepare all threads on all CPUs.
    Inspect the heap of some threads. Check that these are not running.
    """
    self.prepareRunThreads()
    # Inspect heap for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'heap'), [0], (self.threadQuantity + 1) * cpuCount, 0)
    # ~ self.printStdOutStdErr(lines)

  @pytest.mark.thread8
  def testHeapSingleThreadDecimal(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread)

  @pytest.mark.thread8
  def testHeapSingleThreadHex(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}')

  @pytest.mark.thread32
  def testHeapSingleThreadDecimal32(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    self.threadQuantity = 32
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread)

  @pytest.mark.thread32
  def testHeapSingleThreadHex32(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    self.threadQuantity = 32
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}')

  def runThreadXCommand(self, cpu, thread):
    """Test for one thread. If commandSet=True set the time (parameter) with the command.
    In all cases, read this value. Check the output of both commands.
    """
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'heap'), [0], self.threadQuantity + 1, 0)
