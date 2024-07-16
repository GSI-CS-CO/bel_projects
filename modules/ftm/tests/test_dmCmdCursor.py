import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'cursor'.
Main focus is testing with bit masks for CPUs and threads.
"""
class CursorTests(dm_testbench.DmTestbench):

  def testInspectCursor(self):
    """Prepare all threads on all CPUs.
    Inspect the cursor of some threads. Since the output is one line per
    CPU and thread, we can only count the output lines.
    """
    self.prepareRunThreads()
    # run cursor for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'cursor'), [0], threadCount * cpuCount, 0)
