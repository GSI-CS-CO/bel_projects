import dm_testbench

"""Class collects unit tests for prio and type.
"""
class UnitTestPrioType(dm_testbench.DmTestbench):

  def test_static_prio_and_type_reltime(self):
    """Load the schedule
    """
    self.addSchedule('static-prio_and_type-schedule.dot')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i', self.schedulesFolder + 'static-prio_and_type-command-rel.dot'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'static-prio_and_type-expected-rel-0.txt', excludeField='VTIME')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_IN1'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'static-prio_and_type-expected-rel-1.txt', excludeField='VTIME')

  def test_static_prio_and_type_abstime(self):
    """Load the schedule
    """
    self.addSchedule('static-prio_and_type-schedule.dot')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i', self.schedulesFolder + 'static-prio_and_type-command-abs.dot'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'static-prio_and_type-expected-abs-0.txt', excludeField='VTIME')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_IN1'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'static-prio_and_type-expected-abs-1.txt', excludeField='VTIME')
