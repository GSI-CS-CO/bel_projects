import dm_testbench

"""Class collects unit tests for loop.
"""
class UnitTestStartStopAbort(dm_testbench.DmTestbench):

  def test_dynamic_start_abort(self):
    """Load the schedule dynamic-basic-start_stop_abort-schedule.dot and start pattern *.
    """
    self.addSchedule('dynamic-basic-start_stop_abort-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-1.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-3.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'abortpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-5.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-6.txt')

  def test_dynamic_start_stop(self):
    """Load the schedule dynamic-basic-start_stop_abort-schedule.dot and start pattern *.
    """
    self.addSchedule('dynamic-basic-start_stop_abort-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-1.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-0-3.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-1-5.txt')
    self.delay(1.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-1-6.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-basic-start_stop_abort-expected-1-7.txt')

