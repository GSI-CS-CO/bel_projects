import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'asyncClear'.
Main focus is testing with bit masks for CPUs and threads.

Tests are prepared for 8 threads and 32 threads in lm32 firmware.
"""
class AsyncClearTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testInspectAsyncClear(self):
    self.runInspectAsyncClearThreads()

  @pytest.mark.thread32
  def testInspectAsyncClear32(self):
    self.threadQuantity = 32
    self.runInspectAsyncClearThreads()

  def runInspectAsyncClearThreads(self):
    """Prepare all threads on all CPUs.
    Run the asyncClear for some threads. This needs a block with a
    queue. There is no output of the command.
    """
    self.prepareRunThreads()
    # run asyncClear for some CPUs and threads.
    cpu = '0x3'
    thread = '0xaa'
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'asyncclear', 'Block0b'), [0], 0, 0)
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
