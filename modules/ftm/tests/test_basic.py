import dm_testbench

"""Class collects basic tests.
1. add an remove a schedule.
"""
class UnitTestBasic(dm_testbench.DmTestbench):

  def test_static_basic(self):
    """Load the schedule static-basic-schedule.dot.
    """
    if self.cpuQuantity == 3:
      if self.threadQuantity == 8:
        fileName = self.schedulesFolder + 'static-basic-expected-1-0-3cpu.txt'
      else:
        fileName = self.schedulesFolder + 'static-basic-expected-1-0-3cpu-thread32.txt'
    else:
      if self.threadQuantity == 8:
        fileName = self.schedulesFolder + 'static-basic-expected-1-0.txt'
      else:
        fileName = self.schedulesFolder + 'static-basic-expected-1-0-thread32.txt'
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'static-basic-expected-0-0.txt')
    self.addSchedule('static-basic-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, fileName)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + 'static-basic-schedule.dot'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'static-basic-expected-0-0.txt')
