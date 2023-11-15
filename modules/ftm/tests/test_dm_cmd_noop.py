import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'noop'.
Main focus is testing with bit masks for CPUs and threads.
"""
class NoopTests(dm_testbench.DmTestbench):

  def testNoop(self):
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
    'Priority 0 (priolo)  RdIdx: 0 WrIdx: 1    Pending: 1']
    for i in range(1,5):
      self.assertEqual(lines[0][i], expectedLine[i], 'wrong output, expected: ' + expectedLine[i])
    expectedText = '#0 pending Valid Time: 0x0000000000000000 0000000000000000000    CmdType: noop    Qty: 1'
    self.assertEqual(lines[0][5], expectedText, 'wrong output, expected: ' + expectedText)
    for i in range(6,9):
      expectedText = '#{variable} empty   -'.format(variable=i-5)
      self.assertEqual(lines[0][i], expectedText, 'wrong output, expected: ' + expectedText)
