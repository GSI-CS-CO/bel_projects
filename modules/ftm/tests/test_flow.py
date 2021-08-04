import dm_testbench

"""Class collects unit tests for flow.
Test the dm-cmd flow command with combinations of -a and -l

ToDo: extract
"""
class UnitTestFlow(dm_testbench.DmTestbench):

  def test_dynamic_branch_single_tvalid_rel_0(self):
    self.startPattern(self.datamaster, 'dynamic-branch-single-schedule.dot', 'IN_C0')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'flowpattern', 'IN_C0', 'B'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-1.txt', delete=[6])
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-3.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-4.txt', delete=[24])

  def test_dynamic_branch_single_tvalid_rel_1(self):
    self.startPattern(self.datamaster, 'dynamic-branch-single-schedule.dot', 'IN_C0')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'flowpattern', 'IN_C0', 'B', '-l', '1000000000'))
    self.delay(1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-1.txt', delete=[6])
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-3.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-4.txt', delete=[24])

  def test_dynamic_branch_single_tvalid_abs_0(self):
    self.startPattern(self.datamaster, 'dynamic-branch-single-schedule.dot', 'IN_C0')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'flowpattern', 'IN_C0', 'B', '-a'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-1.txt', delete=[6])
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-3.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-4.txt', delete=[24])

  def test_dynamic_branch_single_tvalid_abs_1(self):
    self.startPattern(self.datamaster, 'dynamic-branch-single-schedule.dot', 'IN_C0')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'flowpattern', 'IN_C0', 'B', '-l', '1000000000', '-a'))
    self.delay(1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-1.txt', delete=[6])
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-3.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_cmd, self.datamaster, 'rawqueue', 'BLOCK_IN0'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-branch-single-expected-1-4.txt', delete=[24])

