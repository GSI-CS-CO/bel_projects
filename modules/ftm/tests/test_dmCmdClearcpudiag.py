import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'clearcpudiag'.
Main focus is testing with bit masks for CPUs and threads.
"""
class ClearCpuDiagTests(dm_testbench.DmTestbench):

  def testClearCpuDiag(self):
    """Prepare all threads on all CPUs.
    Run clearcpudiag on some threads. There is no output of the command.
    Check the result with dm-cmd ... diag.
    """
    self.prepareRunThreads()
    # run clearcpudiag for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'clearcpudiag'), [0], 0, 0)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'diag'), [0], 32 + 6 * self.cpuQuantity, 0)
    expectedText = ['║   0 │ 1970-01-01 00:00:00.000000000   │          │          │                ? ║',
                    '║   1 │ 1970-01-01 00:00:00.000000000   │          │          │                ? ║']
    for i in [11 + self.cpuQuantity, 14 + 2 * self.cpuQuantity]:
      self.assertEqual(lines[0][i], expectedText[0], f'line {i} wrong output, expected: ' + expectedText[0])
      self.assertEqual(lines[0][i+1], expectedText[1], f'line {i+1} wrong output, expected: ' + expectedText[1])
    # ~ self.printStdOutStdErr(lines)
