import dm_testbench

"""Class collects unit tests for async.
"""
class UnitTestAsync(dm_testbench.DmTestbench):

  def test_dynamic_async(self):
    """Load the schedule dynamic-async-schedule.dot.
    """
    self.startPattern('dynamic-async-schedule.dot', 'LOOP')
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmSched, self.datamaster, 'rawvisited'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-0-3.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i', self.schedulesFolder + 'dynamic-async-command-0a.dot'))
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'showlocks'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-1-1.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i', self.schedulesFolder + 'dynamic-async-command-0b.dot'))
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_B'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-1-3.txt', excludeField='QTY')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i', self.schedulesFolder + 'dynamic-async-command-0c.dot'))
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_B'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-1-5.txt', excludeField='VTIME')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i', self.schedulesFolder + 'dynamic-async-command-0d.dot'))
    self.delay(1.4)
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmSched, self.datamaster, 'rawvisited'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-1-7.txt')
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'showlocks'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-1-8.txt')
    stdoutLines = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'rawqueue', 'BLOCK_B'])
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + 'dynamic-async-expected-1-9.txt', excludeField='VTIME')
