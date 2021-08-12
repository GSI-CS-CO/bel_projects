import dm_testbench

"""Class collects unit tests to run a pattern on a single cpu.
This is tested for all four cpus.
"""
class UnitTestRunAllSingle(dm_testbench.DmTestbench):

  def common_dynamic_run_all_single(self, number):
    """Load the schedule dynamic-basic-run_all_single-schedule.dot.
    Check that no node is visited. Then start pattern IN_C?.
    Check again the visited nodes.
    """
    self.addSchedule('dynamic-basic-run_all_single-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-run_all_single-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'IN_C' + number))
    self.delay(0.1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binary_dm_sched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-run_all_single-expected-' + number + '-2.txt')

  def test_dynamic_run_cpu0(self):
    self.common_dynamic_run_all_single('0')

  def test_dynamic_run_cpu1(self):
    self.common_dynamic_run_all_single('1')

  def test_dynamic_run_cpu2(self):
    self.common_dynamic_run_all_single('2')

  def test_dynamic_run_cpu3(self):
    self.common_dynamic_run_all_single('3')
