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
    # check the result with dm-cmd ... queue Block0b.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'queue', 'Block0b'), [0], 10, 0)
    # verify the output (lines 1 to 9).
    expectedLine = ['', 'Inspecting Queues of Block Block0b',
    'Priority 2 (prioil)  Not instantiated',
    'Priority 1 (priohi)  Not instantiated',
    'Priority 0 (priolo)  RdIdx: 0 WrIdx: 0    Pending: 0']
    for i in range(1,5):
      self.assertEqual(lines[0][i], expectedLine[i], 'wrong output, expected: ' + expectedLine[i])
    for i in range(5,9):
      expectedText = '#{variable} empty   -'.format(variable=i-5)
      messageText = 'wrong output, expected: #{variable} empty   -'.format(variable=i-5)
      self.assertEqual(lines[0][i], expectedText, messageText)

    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', 'Block0b'), [0], 0, 0)

    # check the result with dm-cmd ... queue Block0b.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'queue', 'Block0b'), [0], 10, 0)
    # verify the output (lines 1 to 9).
    expectedLine = ['', 'Inspecting Queues of Block Block0b',
    'Priority 2 (prioil)  Not instantiated',
    'Priority 1 (priohi)  Not instantiated',
    'Priority 0 (priolo)  RdIdx: 0 WrIdx: 4    Pending: 4']
    for i in range(1,5):
      self.assertEqual(lines[0][i], expectedLine[i], 'wrong output, expected: ' + expectedLine[i])
    for i in range(5,9):
      expectedText = '#{variable} pending Valid Time: 0x0000000000000000 0000000000000000000    CmdType: noop    Qty: 1'.format(variable=i-5)
      messageText = 'wrong output, expected: ' + expectedText
      self.assertEqual(lines[0][i], expectedText, messageText)
