import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'force'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class ForceTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testInspectForce(self):
    self.runInspectForceThreads()

  @pytest.mark.thread32
  def testInspectForce32(self):
    self.threadQuantity = 32
    self.runInspectForceThreads()

  def runInspectForceThreads(self):
    """Prepare all threads on all CPUs.
    Run dm-cmd force on some threads. This forces the thread cursor to
    the value of the corresponding origin. Currently no real check for
    success of the command.
    """
    self.prepareRunThreads()
    # run force for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'force'), [0], 0, 0)
