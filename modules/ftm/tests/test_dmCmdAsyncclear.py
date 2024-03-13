import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'asyncClear'.
Main focus is testing with bit masks for CPUs and threads.
"""
class AsyncClearTests(dm_testbench.DmTestbench):

  def testInspectAsyncClear(self):
    """Prepare all threads on all CPUs.
    Run the asyncClear for some threads. This needs a block with a
    queue. There is no output of the command.
    """
    self.prepareRunThreads()
    # run asyncClear for some CPUs and threads.
    cpu = '0x3'
    # Enable at most 4 threads. Otherwise the command queue is full and command fails.
    thread = '0xaa'
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', 'Block0b'), [0], 0, 0)
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
