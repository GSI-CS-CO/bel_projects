import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'origin'.
Main focus is testing with bit masks for CPUs and threads.
"""
class OriginTests(dm_testbench.DmTestbench):

  def testInspectOrigin(self):
    """Prepare all threads on all CPUs.
    Display the origin of some threads.
    """
    self.prepareRunThreads()
    # run origin for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'origin'), [0], threadCount * cpuCount, 0)
