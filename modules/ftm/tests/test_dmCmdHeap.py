import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'heap'.
Main focus is testing with bit masks for CPUs and threads.
"""
class HeapTests(dm_testbench.DmTestbench):

  def testInspectHeap(self):
    """Prepare all threads on all CPUs.
    Inspect the heap of some threads. Check that these are not running.
    """
    self.prepareRunThreads()
    # Inspect heap for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'heap'), [0], (self.threadQuantity + 1) * cpuCount, 0)
    # ~ self.printStdOutStdErr(lines)

  def testHeapSingleThreadDecimal(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread)

  def testHeapSingleThreadHex(self):
    """Loop for all threads aborting this thread.
    Uses the thread number in hexadecimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}')

  def runThreadXCommand(self, cpu, thread):
    """Test for one thread. Check the number of output lines.
    """
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'heap'), [0], self.threadQuantity + 1, 0)
