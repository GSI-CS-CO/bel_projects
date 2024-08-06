import dm_testbench    # contains super class
import os
import logging

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
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'queue', 'Block0b'), [0], 10, 0)
    self.verifyInspectingQueueOutput(lines, read=0, write=0, pending=0)
    # send a command (here: noop) to Block0b
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', 'Block0b'), [0], 0, 0)
    # check the result with dm-cmd ... queue Block0b.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'queue', 'Block0b'), [0], 10, 0)
    try:
      self.verifyInspectingQueueOutput(lines, read=0, write=1, pending=1)
    except AssertionError as instance:
      testName = os.environ['PYTEST_CURRENT_TEST']
      logging.getLogger().warning(f'{testName}, AssertionError while Inspecting Queues, try second inspection.' + '\n' + '\n'.join(lines))
      self.assertEqual(instance.args[0][:111], "'Priority 0 (priolo)  RdIdx: 0 WrIdx: 1    Pending: 1' != 'Priority 0 (priolo)  RdIdx: 0 WrIdx: 1    Pending: 0", 'wrong exception')
      # second try: check the result with dm-cmd ... queue Block0b.
      self.delay(1.0)
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'queue', 'Block0b'), [0], 10, 0)
      self.verifyInspectingQueueOutput(lines, read=1, write=1, pending=0)

  def verifyInspectingQueueOutput(self, lines, read=0, write=0, pending=0):
    # verify the output (lines 1 to 9).
    expectedLines = ['', 'Inspecting Queues of Block Block0b',
    'Priority 2 (prioil)  Not instantiated',
    'Priority 1 (priohi)  Not instantiated',
    f'Priority 0 (priolo)  RdIdx: {read} WrIdx: {write}    Pending: {pending}',
    '', '', '', '']
    for i in range(0,4):
      expectedLines[i+5] = f'#{(i+read) % 4} empty   -'
    if pending == 1:
      expectedLines[5] = '#0 pending Valid Time: 0x0000000000000000 0000000000000000000    CmdType: noop    Qty: 1'
    for i in range(1,9):
      self.assertEqual(lines[i], expectedLines[i], 'wrong output, expected: ' + expectedLines[i] + '\n' + '\n'.join(lines))
