import dm_testbench    # contains super class

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
    blockNode = 'Block0b'
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    # check the result with dm-cmd ... queue Block0b.
    # The queues should be empty.
    self.inspectQueue(blockNode, read=0, write=0, pending=0)
    # send a command (here: noop) to Block0b
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', blockNode), [0], 0, 0)
    # check the result with dm-cmd ... queue Block0b.
    # The noop command should be pending (pending=1). If this is false,
    # retry.
    self.inspectQueue(blockNode, read=0, write=1, pending=1, retry=True)
