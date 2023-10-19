import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'clearcpudiag'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class ClearCpuDiagTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testClearCpuDiag(self):
    self.runClearCpuDiagThreads()

  @pytest.mark.thread32
  def testClearCpuDiag32(self):
    self.threadQuantity = 32
    self.runClearCpuDiagThreads()

  def runClearCpuDiagThreads(self):
    """Prepare all threads on all CPUs.
    Run clearcpudiag on some threads. There is no output of the command.
    Check the result with dm-cmd ... diag.
    """
    self.prepareRunThreads()
    # run clearcpudiag for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'clearcpudiag'), [0], 0, 0)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'diag'), [0], 56, 0)
    expectedText = ['║   0 │ 1970-01-01 00:00:00.000000000   │          │          │                ? ║',
                    '║   1 │ 1970-01-01 00:00:00.000000000   │          │          │                ? ║']
    for i in [15, 22]:
      self.assertEqual(lines[0][i], expectedText[0], f'{i} wrong output, expected: ' + expectedText[0])
      self.assertEqual(lines[0][i+1], expectedText[1], f'{i+1} wrong output, expected: ' + expectedText[1])
    # ~ self.printStdOutStdErr(lines)
