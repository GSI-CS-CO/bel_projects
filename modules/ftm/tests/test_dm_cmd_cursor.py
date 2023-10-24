import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'cursor'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class CursorTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testInspectCursor(self):
    self.runInspectCursorThreads()

  @pytest.mark.thread32
  def testInspectCursor32(self):
    self.threadQuantity = 32
    self.runInspectCursorThreads()

  def runInspectCursorThreads(self):
    """Prepare all threads on all CPUs.
    Inspect the cursor of some threads. Since the output is one line per
    CPU and thread, we can only count the output lines.
    """
    self.prepareRunThreads()
    # run cursor for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'cursor'), [0], threadCount * cpuCount, 0)
