import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'force'.
Main focus is testing with bit masks for CPUs and threads.
"""
class ForceTests(dm_testbench.DmTestbench):

  def testInspectForce(self):
    """Prepare all threads on all CPUs.
    Run dm-cmd force on some threads. This forces the thread cursor to
    the value of the corresponding origin. Currently no real check for
    success of the command.
    """
    self.prepareRunThreads()
    # run force for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'force'), [0], 0, 0)
