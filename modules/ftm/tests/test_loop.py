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
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-loop-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    if self.threadQuantity == 8:
      fileName0 = self.schedulesFolder + 'dynamic-loop-expected-0-1.txt'
      fileName1 = self.schedulesFolder + 'dynamic-loop-expected-1-1.txt'
      if self.cpuQuantity == 3:
        fileName0 = fileName0.replace('.txt', '-3cpu.txt')
        fileName1 = fileName1.replace('.txt', '-3cpu.txt')
    else:
      fileName0 = self.schedulesFolder + 'dynamic-loop-expected-0-1-thread32.txt'
      fileName1 = self.schedulesFolder + 'dynamic-loop-expected-1-1-thread32.txt'
    self.compareExpectedOutput(stdoutLines, fileName0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_B'))
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-loop-expected-1-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, fileName1)
