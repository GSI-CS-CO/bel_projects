import dm_testbench

"""Class collects unit tests for loop.
"""
class UnitTestLoop(dm_testbench.DmTestbench):

  def test_dynamic_loop(self):
    """Load the schedule dynamic-loop-schedule.dot and start pattern IN_A.
    Check the visited nodes (rawvisited) and rawstatus.
    Then start pattern IN_B and check the visited nodes (rawvisited) and rawstatus again.
    """
    self.startPattern('dynamic-loop-schedule.dot', 'IN_A')
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-0-1.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN_B'))
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-1-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-1-1.txt')
