import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'origin'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class OriginTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testInspectOrigin(self):
    self.runInspectOriginThreads()

  @pytest.mark.thread32
  def testInspectOrigin32(self):
    self.threadQuantity = 32
    self.runInspectOriginThreads()

  def runInspectOriginThreads(self):
    """Prepare all threads on all CPUs.
    Display the origin of some threads.
    """
    self.prepareRunThreads()
    # run origin for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'origin'), [0], threadCount * cpuCount, 0)
