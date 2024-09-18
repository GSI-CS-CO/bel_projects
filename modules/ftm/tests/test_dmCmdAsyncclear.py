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

    First check that the noop comand is in the low prio queue (pending=1) and read=0 and write=1.
    Second check that the queue is cleared. This is pending=0, write=0, read=0 (indices are reset).
    If this fails, the queue is not cleared (read=0, write=1, pending=1) or
    the noop command is executed (read=1, write=1, pending=0).
    """
    self.prepareRunThreads()
    # run asyncClear for some CPUs and threads.
    cpu = '0x3'
    # Enable at most 4 threads. Otherwise the command queue is full and command fails.
    thread = '0xaa'
    blockNode = 'Block0b'
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'noop', blockNode), [0], 0, 0)
    # The noop command should be pending (pending=1).
    try:
      self.inspectQueue(blockNode, read=0, write=1, pending=1)
    except AssertionError as errorInstance:
      # sometimes the noop command is already executed. Then read and write index are 1 and pending is 0.
      self.inspectQueue(blockNode, read=1, write=1, pending=0)
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'asyncclear', blockNode), [0], 0, 0)
    # check the result with dm-cmd ... queue Block0b.
    # The queues should be empty and the indices 0 after asynchronous clear.
    self.inspectQueue(blockNode, read=0, write=0, pending=0, retry=True)
