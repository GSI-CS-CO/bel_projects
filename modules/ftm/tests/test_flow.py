import dm_testbench

"""Class collects unit tests for flow.
Test the dm-cmd flow command with combinations of -a and -l
"""
class UnitTestFlow(dm_testbench.DmTestbench):

  def test_dynamic_branch_single(self):
    self.startPattern(self.datamaster, 'dynamic-branch-single-schedule.dot', 'IN_C0')
    stdoutLines = self.startAndGetSubprocessStdout(('dm-sched', self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-0-0.txt')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flowpattern', 'IN_C0', 'B'))
    stdoutLines = self.startAndGetSubprocessStdout(('dm-cmd', self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-1.txt')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout(('dm-sched', self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-3.txt')
    stdoutLines = self.startAndGetSubprocessStdout(('dm-cmd', self.datamaster, 'rawqueue BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-4.txt')

