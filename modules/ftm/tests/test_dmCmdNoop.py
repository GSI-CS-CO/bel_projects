import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'noop'.
Main focus is testing with bit masks for CPUs and threads.
"""
class NoopTests(dm_testbench.DmTestbench):

  def testNoop(self):
    """Prepare all threads on all CPUs.
    Run dm-cmd noop on some threads.

    First check that the low prio queue is empty (pending=0) and read index and write index are 0.
    Second check that the noop command is in the queue. This is read=0, write=1, pending=1.
    If this fails, the noop command is not yet in the queue (read=0, write=0, pending=0) or
    the noop command is executed (read=1, write=1, pending=0).
    """
    self.prepareRunThreads()
    # run noop for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    blockNode = 'Block0b'
    # check the result with dm-cmd ... queue Block0b.
    # The queues should be empty.
    self.inspectQueue(blockNode, read=0, write=0, pending=0)
    # send a command (here: noop) to Block0b
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', blockNode), [0], 0, 0)
    # check the result with dm-cmd ... queue Block0b.
    # The noop command should be pending (pending=1). If this is false,
    # retry.
    try:
      self.inspectQueue(blockNode, read=0, write=1, pending=1, retry=True)
    except AssertionError as errorInstance:
      # sometimes the noop command is already executed. Then read and write index are 1 and pending is 0.
      self.inspectQueue(blockNode, read=1, write=1, pending=0)
