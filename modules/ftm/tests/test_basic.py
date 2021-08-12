import dm_testbench

"""Class collects basic tests.
1. add an remove a schedule.
"""
class UnitTestBasic(dm_testbench.DmTestbench):

  def test_static_basic(self):
    """Load the schedule static-basic-schedule.dot.
    """
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-0-0.txt')
    self.addSchedule('static-basic-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-1-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'remove', self.schedules_folder + 'static-basic-schedule.dot'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-0-0.txt')
