import dm_testbench

"""Class collects unit tests for loop.
"""
class UnitTestStartStopAbort(dm_testbench.DmTestbench):

  def test_dynamic_start_abort(self):
    """Load the schedule dynamic-basic-start_stop_abort-schedule.dot and start pattern *.
    """
    if self.threadQuantity == 8:
      fileName1 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-1.txt'
      fileName3 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-3.txt'
      fileName5 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-5.txt'
    else:
      fileName1 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-1-thread32.txt'
      fileName3 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-3-thread32.txt'
      fileName5 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-5-thread32.txt'
    self.addSchedule('dynamic-basic-start_stop_abort-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName3)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'abortpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-6.txt')

  def test_dynamic_start_stop(self):
    """Load the schedule dynamic-basic-start_stop_abort-schedule.dot and start pattern *.
    """
    if self.threadQuantity == 8:
      fileName01 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-1.txt'
      fileName03 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-3.txt'
      fileName15 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-1-5.txt'
      fileName16 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-1-6.txt'
    else:
      fileName01 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-1-thread32.txt'
      fileName03 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-3-thread32.txt'
      fileName15 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-1-5-thread32.txt'
      fileName16 = self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-1-6-thread32.txt'
    self.addSchedule('dynamic-basic-start_stop_abort-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName01)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName03)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'IN_C0'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName15)
    self.delay(1.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName16)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-basic-start_stop_abort-expected-1-7.txt')
