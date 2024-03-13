import dm_testbench

"""Class collects unit tests for the command line of dm-cmd.
First section: all commands which need a target name. Test case names: test_<command>_missing, test_<command>.

Prerequisite: datamaster must have a node with name B_PPS. Therefore, dm_cmd.dot is started.
"""
class TestDmCmd(dm_testbench.DmTestbench):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    TestDmCmd.dmCmdPatternStarted = False


  def setUp(self):
    """Set up for all test cases in this class: start dm_cmd pattern.
    """
    if not TestDmCmd.dmCmdPatternStarted:
      self.initDatamaster()
      self.startPattern('dm_cmd.dot')
      TestDmCmd.dmCmdPatternStarted = True

  def targetName_missing(self, command):
    """Common method for test cases with missing target name.
    Start dm-cmd with the command and check the output on stderr.
    """
    lines = self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, command],
         expectedReturnCode=[255], linesCout=0, linesCerr=1)[1]
    test_result = False
    for line in lines:
      if 'Target node is NULL, target missing.' in line:
        test_result = True
    self.assertTrue(test_result, f'stderr: {lines}')

  def test_abswait_missing(self):
    self.targetName_missing('abswait')

  def test_asyncclear_missing(self):
    self.targetName_missing('asyncclear')

  def test_flow_missing(self):
    self.targetName_missing('flow')

  def test_flush_missing(self):
    self.targetName_missing('flush')

  def test_lock_missing(self):
    self.targetName_missing('lock')

  def test_noop_missing(self):
    self.targetName_missing('noop')

  def test_queue_missing(self):
    self.targetName_missing('queue')

  def test_rawqueue_missing(self):
    self.targetName_missing('rawqueue')

  def test_relwait_missing(self):
    self.targetName_missing('relwait')

  def test_staticflush_missing(self):
    self.targetName_missing('staticflush')

  def test_switch_missing(self):
    self.targetName_missing('switch')

  def test_unlock_missing(self):
    self.targetName_missing('unlock')

  def targetName(self, command, expectedReturnCode=[0], count_out=0, count_err=0, options=''):
    """Common method for commands with target name.
    Start dm-cmd with the command and check the output on stdout and stderr.
    The checks check only the number of lines of the output, not the content.
    """
    lines, lineErr = self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, command, 'B_PPS', options],
         expectedReturnCode, linesCout=count_out, linesCerr=count_err)

  def test_abswait(self):
    self.targetName('abswait', options='100')

  def test_asyncclear(self):
    self.targetName('asyncclear')

  def test_flow(self):
    self.targetName('flow', [255], count_err=1, options='x')

  def test_flush(self):
    self.targetName('flush', [255], count_err=1)

  def test_lock(self):
    self.targetName('lock')

  def test_noop(self):
    self.targetName('noop')

  def test_queue(self):
    self.targetName('queue', count_out=10)

  def test_rawqueue(self):
    self.targetName('rawqueue', count_out=30)

  def test_relwait(self):
    self.targetName('relwait', options='100')

  def test_staticflush(self):
    self.targetName('staticflush', [255], count_err=1)

  def test_switch(self):
    self.targetName('switch', [255], count_err=1, options='x')

  def test_unlock(self):
    self.targetName('unlock')

"""Class collects unit tests for the command line of dm-cmd.
Tested commands are:
dm-cmd reset
dm-cmd reset all
"""
class TestDmCmdOther(dm_testbench.DmTestbench):
  def test_reset(self):
    """Test 'reset' command
    """
    self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'reset'],
         [0], linesCout=1, linesCerr=0)

  def test_reset_all(self):
    """Test 'reset all' command
    """
    self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'reset', 'all'],
         [0], linesCout=1, linesCerr=0)

"""Class collects unit tests for the dm-cmd stop command.

Prerequisite: datamaster must have a node with name B_PPS. Therefore, dm_cmd.dot is started.
"""
class TestDmCmdStop(dm_testbench.DmTestbench):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    TestDmCmdStop.dmCmdPatternStarted = False


  def setUp(self):
    """Set up for all test cases in this class: start dm_cmd pattern.
    """
    if not TestDmCmdStop.dmCmdPatternStarted:
      self.initDatamaster()
      self.startPattern('dm_cmd_stop.dot')
      TestDmCmdStop.dmCmdPatternStarted = True

  def test_dm_cmd_stop_node(self):
    """Start dm-cmd with the stop command and a block which has no queue.
    Test for the correct error message.
    """
    linesErr = self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'stop', 'B_PPS1'],
         expectedReturnCode=[0], linesCout=0, linesCerr=1)[1]
    self.assertTrue("Target node 'B_PPS1' was not found on DM" in linesErr[0])

  def test_dm_cmd_stop_block(self):
    """Start dm-cmd with the stop command and a block with a low prio queue.
    Test is ok (stops the pattern).
    """
    self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'stop', 'B_PPS'],
         expectedReturnCode=[0], linesCout=0, linesCerr=0)

  def test_dm_cmd_stop_block_fail(self):
    """Start dm-cmd with the stop command and a block which has no queue.
    Test for the correct error message.
    """
    linesErr = self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'stop', 'B_PPSx'],
         expectedReturnCode=[255], linesCout=0, linesCerr=1)[1]
    self.assertTrue("Block node 'B_PPSx' does not have a low prio queue." in linesErr[0])

  def test_dm_cmd_stop(self):
    """Start dm-cmd with the stop command, but no target name.
    Test for the correct error message.
    """
    linesErr = self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'stop'],
         expectedReturnCode=[0], linesCout=0, linesCerr=1)[1]
    self.assertTrue("Target name is missing" in linesErr[0])

  def test_dm_cmd_stop_event(self):
    """Start dm-cmd with the stop command and an event.
    Test for the correct error message.
    """
    linesErr = self.startAndGetSubprocessOutput([self.binaryDmCmd, self.datamaster, 'stop', 'EVT_PPS1'],
         expectedReturnCode=[255], linesCout=0, linesCerr=1)[1]
    self.assertTrue("Node 'EVT_PPS1' is not a block" in linesErr[0])

