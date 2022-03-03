import dm_testbench
from datetime import datetime as dt

"""
Class collects unit tests for parallelBranch.
Main test case is:
Add a schedule to a clear datamaster and start the patterns.
Branch the flow on different blocks at the same time to an alternate destination.
"""
class UnitTestParallelBranch(dm_testbench.DmTestbench):
  offsetNanoseconds1 = 300000000 # offsets in nanoseconds, should be between 0 and 1 second.
  offsetNanoseconds2 = 700000000
  perNanosecond = 1000000000 # 10**9

  def doBranch(self):
    if self.diag:
      diag1 = self.startAndGetSubprocessStdout([self.binary_dm_cmd, self.datamaster, 'diag'])
    output = self.startAndGetSubprocessStdout([self.binary_dm_cmd, self.datamaster, 'deadline'])
    iDeadline = int(output[0][21:])
    offsetNanosecondsStr1 = "{:0d}".format(iDeadline + self.offsetNanoseconds1)
    for x in self.cpuList:
      self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr1, 'Block' + x, x + '2'))
    if self.diag:
      diag2 = self.startAndGetSubprocessStdout([self.binary_dm_cmd, self.datamaster, 'diag'])
    offsetNanosecondsStr2 = "{:0d}".format(iDeadline + self.offsetNanoseconds2)
    for x in self.cpuList:
      self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr2, 'Block' + x, x + '3'))
    if self.diag:
      diag3 = self.startAndGetSubprocessStdout([self.binary_dm_cmd, self.datamaster, 'diag'])
    print(f'\nUTC deadline: {dt.utcfromtimestamp(iDeadline/self.perNanosecond)}, \
      UTC deadline + {self.offsetNanoseconds1/self.perNanosecond:8.6f}: {dt.utcfromtimestamp((int(output[0][21:]) + self.offsetNanoseconds1)/self.perNanosecond)}, \
      UTC deadline + {self.offsetNanoseconds2/self.perNanosecond:8.6f}: {dt.utcfromtimestamp((int(output[0][21:]) + self.offsetNanoseconds2)/self.perNanosecond)}')
    # write here because other operations are time critical
    if self.diag:
      with open('diag1_branch.txt', 'w') as file1:
        file1.write("\n".join(diag1))
      with open('diag2_branch.txt', 'w') as file2:
        file2.write("\n".join(diag2))
      with open('diag3_branch.txt', 'w') as file3:
        file3.write("\n".join(diag3))

  def run_test_branch(self, cpuList, diag=False):
    file_name = 'snoop_branch1.csv'
    self.diag = diag
    self.cpuList = cpuList
    self.snoopTime = int(max(self.offsetNanoseconds1, self.offsetNanoseconds2)/1000000000) + 1
    self.addSchedule('branch1.dot')
    for x in cpuList:
      self.startPattern('', x)
    self.snoopToCsvWithAction(file_name, self.doBranch, self.snoopTime)
    check_values = {}
    column_evtno = 8
    if len(cpuList) == 1:
      check_values = {'0x00a1': '>990', '0x00a2': '1', '0x00a3': '1'}
    elif len(cpuList) == 2:
      check_values = {'0x00a1': '>990', '0x00a2': '1', '0x00a3': '1',
                      '0x00b1': '>990', '0x00b2': '1', '0x00b3': '1'}
    elif len(cpuList) == 3:
      check_values = {'0x00a1': '>990', '0x00a2': '1', '0x00a3': '1',
                      '0x00b1': '>990', '0x00b2': '1', '0x00b3': '1',
                      '0x00c1': '>990', '0x00c2': '1', '0x00c3': '1'}
    elif len(cpuList) == 4:
      check_values = {'0x00a1': '>990', '0x00a2': '1', '0x00a3': '1',
                      '0x00b1': '>990', '0x00b2': '1', '0x00b3': '1',
                      '0x00c1': '>990', '0x00c2': '1', '0x00c3': '1',
                      '0x00d1': '>990', '0x00d2': '1', '0x00d3': '1'}
    self.analyseFrequencyFromCsv(file_name, column_evtno, check_values=check_values)
    self.deleteFile(file_name)

  def test_branchCPU0(self):
    self.run_test_branch(['A'])

  def test_branchCPU01(self):
    self.run_test_branch(['A', 'B'])

  def test_branchCPU012(self):
    self.run_test_branch(['A', 'B', 'C'])

  def test_branchCPU0123(self):
    self.run_test_branch(['A', 'B', 'C', 'D'])
