import dm_testbench

"""Class collects unit tests for loop.
"""
class UnitTestRunCpu0Single(dm_testbench.DmTestbench):

  def test_dynamic_run_cpu0_single(self):
    """Load the schedule dynamic-basic-run_cpu0_single-schedule.dot and start pattern *.
    """
    self.addSchedule('dynamic-basic-run_cpu0_single-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-run_cpu0_single-expected-0-0.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN0'))
    self.delay(0.1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-run_cpu0_single-expected-1-1.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN1'))
    self.delay(0.1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-run_cpu0_single-expected-2-1.txt')
