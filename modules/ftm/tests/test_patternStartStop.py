import dm_testbench

"""
Test dm-cmd startpattern and dm-cmd stoppattern.
"""
class TestStartPatternStopPattern(dm_testbench.DmTestbench):

  def test_startStopPattern(self):
    """Run a pps pattern on all threads of CPU 0.
    Stop pattern Block0b which should fail, since not a pattern name.
    Stop pattern PPS0b which is OK.
    Start pattern Block0b which should fail, since not a pattern name.
    Start pattern PPS0b which is OK.
    """
    # Start pps pattern on all threads of CPU 0.
    self.prepareRunThreads(cpus=1);
    lines = ['']
    outputErr = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, 'stoppattern', 'Block0b'), [255], 0, 1)[1]
    self.assertEqual(outputErr[0], "../bin/dm-cmd: Target 'Block0b' is not a pattern name.")
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'stoppattern', 'PPS0b'), [0], 0, 0)
    # A delay is needed since the block has to be executed once to process the stop command.
    # Since the block in the pps pattern has a period of 1 second, the delay is 1 second.
    self.delay(1.0)
    lines[0] = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0x1', 'running'), [0], 1, 0)[0]
    threadState = ''
    if self.threadQuantity > 8:
      threadState = self.checkRunningThreads(lines, ['0xfdffffff', '0xfffdffff', '0xfffffdff', '0xfffffffd'])
      if threadState == '0xfdffffff':
        thread = '25'
      elif threadState == '0xfffdffff':
        thread = '17'
      elif threadState == '0xfffffdff':
        thread = '9'
      elif threadState == '0xfffffffd':
        thread = '1'
    else:
      self.checkRunningThreads(lines, ['0xfd'])
      thread = '1'
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'startpattern', 'Block0b'), [255], 1, 1)
    self.assertEqual(outputErr[0], "../bin/dm-cmd: Target 'Block0b' is not a pattern name.")
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-t', thread, 'startpattern', 'PPS0b'), [0], 1, 0)
    lines[0] = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0x1', 'running'), [0], 1, 0)[0]
    if self.threadQuantity > 8:
      self.checkRunningThreads(lines, ['0xffffffff'])
    else:
      self.checkRunningThreads(lines, ['0xff'])
