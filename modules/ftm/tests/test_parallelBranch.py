import dm_testbench
from datetime import datetime as dt

"""
Class collects unit tests for parallelBranch.
Main test case is:
Add a schedule to a clear datamaster and start the patterns.
Branch the flow on different blocks at the same time to an alternate destination.
"""
class UnitTestParallelBranch(dm_testbench.DmTestbench):

  def doBranch1(self):
    output = self.startAndGetSubprocessStdout(['dm-cmd', self.datamaster, 'deadline'])
    offsetNanosecondsStr1 = "{:0d}".format(int(output[0][21:]) + 500000000)
    offsetNanosecondsStr2 = "{:0d}".format(int(output[0][21:]) + 700000000)
    print()
    print(f'{offsetNanosecondsStr1} {offsetNanosecondsStr2}')
    print(f'{dt.fromtimestamp((int(output[0][21:]) + 500000000)/1000000000)} {dt.fromtimestamp((int(output[0][21:]) + 700000000)/1000000000)}')
    print(f'{dt.utcfromtimestamp((int(output[0][21:]) + 500000000)/1000000000)} {dt.utcfromtimestamp((int(output[0][21:]) + 700000000)/1000000000)}')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr1, 'BlockA', 'A2'))
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr1, 'BlockB', 'B2'))
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr2, 'BlockA', 'A3'))
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr2, 'BlockB', 'B3'))

  def test_branch1(self):
    self.startAllPattern(self.datamaster, 'branch1.dot')
    file_name = 'snoop_branch1.csv'
    parameter_column = 20
    self.snoopToCsvWithAction(file_name, self.doBranch1, 1)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    print(self.startAndGetSubprocessStdout(('grep', '-nHm', '1', 'a1', file_name)))
    print(self.startAndGetSubprocessStdout(('grep', '-nHm', '1', 'b1', file_name)))
    print(self.startAndGetSubprocessStdout(('grep', '-nH', 'a2', file_name)))
    print(self.startAndGetSubprocessStdout(('grep', '-nH', 'b2', file_name)))
    print(self.startAndGetSubprocessStdout(('grep', '-nH', 'a3', file_name)))
    print(self.startAndGetSubprocessStdout(('grep', '-nH', 'b3', file_name)))
    self.deleteFile(file_name)
