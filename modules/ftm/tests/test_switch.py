import dm_testbench

"""Class collects unit tests for switch.
"""
class UnitTestSwitch(dm_testbench.DmTestbench):

  def test_dynamic_switch(self):
    """Load the schedule dynamic-switch-schedule.dot.
    Check the visited nodes (rawvisited).
    """
    self.addSchedule('dynamic-switch-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-switch-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN0'))
    self.delay(0.1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-switch-expected-1-1.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN1'))
    self.delay(0.1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-switch-expected-2-1.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, '-i', self.schedules_folder + 'dynamic-switch-command.dot'))
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-switch-expected-3-1.txt')
