import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'noop'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class NoopTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testNoop(self):
    self.runNoopThreads()

  @pytest.mark.thread32
  def testNoop32(self):
    self.threadQuantity = 32
    self.runNoopThreads()

  def runNoopThreads(self):
    """Prepare all threads on all CPUs.
    Run dm-cmd noop on some threads.
    """
    self.prepareRunThreads()
    # run noop for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', 'Block0b'), [0], 0, 0)
