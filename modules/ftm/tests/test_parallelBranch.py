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
    self.delay(0.1)
    if self.diag:
      diag1 = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'diag'])
    output = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'deadline'])
    iDeadline = int(output[0][21:])
    offsetNanosecondsStr1 = "{:0d}".format(iDeadline + self.offsetNanoseconds1)
    for x in self.cpuList:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr1, 'Block' + x, x + '2'))
    if self.diag:
      diag2 = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'diag'])
    offsetNanosecondsStr2 = "{:0d}".format(iDeadline + self.offsetNanoseconds2)
    for x in self.cpuList:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'flow', '-q', '1', '-a', '-l', offsetNanosecondsStr2, 'Block' + x, x + '3'))
    if self.diag:
      diag3 = self.startAndGetSubprocessStdout([self.binaryDmCmd, self.datamaster, 'diag'])
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

  def generateSchedule(self, scheduleFile, cpuList):
    """Generate the following schedule and write it to a file. For each
    CPU a section with names containing 'A', 'B', ... ist generated.
        digraph branch1 {
          name=branch1
          node [type=tmsg cpu=0 fid=1 toffs=0 pattern=A]
          A1 [par="0xA1" evtno="0xA1"]
          A2 [par="0xA2" evtno="0xA2"]
          A3 [par="0xA3" evtno="0xA3"]
          BlockA [type=block qlo=1 tperiod=1000000 patentry=1 patexit=1]
          A2 -> BlockA -> A1 -> BlockA [type=defdst]
          A3 -> BlockA [type=defdst]
          BlockA -> A2 [type=altdst]
          BlockA -> A3 [type=altdst]
          node [type=tmsg cpu=1 fid=1 toffs=0 pattern=B]
          B1 [par="0xB1" evtno="0xB1"]
          B2 [par="0xB2" evtno="0xB2"]
          B3 [par="0xB3" evtno="0xB3"]
          BlockB [type=block qlo=1 tperiod=1000000 patentry=1 patexit=1]
          B2 -> BlockB -> B1 -> BlockB [type=defdst]
          B3 -> BlockB [type=defdst]
          BlockB -> B2 [type=altdst]
          BlockB -> B3 [type=altdst]
          """
    lines = []
    lines.append(f'digraph branch1 ' + '{')
    lines.append(f'  name=branch1')
    for patternName in cpuList:
      lines.append(f'  node [type=tmsg cpu={ord(patternName) - ord("A")} fid=1 toffs=0 pattern={patternName}]')
      lines.append(f'  {patternName}1 [par="0x{patternName}1" evtno="0x{patternName}1"]')
      lines.append(f'  {patternName}2 [par="0x{patternName}2" evtno="0x{patternName}2"]')
      lines.append(f'  {patternName}3 [par="0x{patternName}3" evtno="0x{patternName}3"]')
      lines.append(f'  Block{patternName} [type=block qlo=1 tperiod=1000000 patentry=1 patexit=1]')
      lines.append(f'  {patternName}2 -> Block{patternName} -> {patternName}1 -> Block{patternName} [type=defdst]')
      lines.append(f'  {patternName}3 -> Block{patternName} [type=defdst]')
      lines.append(f'  Block{patternName} -> {patternName}2 [type=altdst]')
      lines.append(f'  Block{patternName} -> {patternName}3 [type=altdst]')
    lines.append('}')
    lines.append('')
    # write the file
    with open(scheduleFile, 'w') as file1:
      file1.write("\n".join(lines))

  def run_test_branch(self, cpuList, diag=False):
    fileName = 'snoop_branch1.csv'
    scheduleFile = 'branch1.dot'
    self.diag = diag
    self.cpuList = cpuList
    while len(self.cpuList) > self.cpuQuantity:
      self.cpuList.pop()
    self.snoopTime = int(max(self.offsetNanoseconds1, self.offsetNanoseconds2)/1000000000) + 1
    self.generateSchedule(self.schedulesFolder + scheduleFile, self.cpuList)
    self.addSchedule(scheduleFile)
    for x in cpuList:
      self.startPattern('', x)
    self.snoopToCsvWithAction(fileName, self.doBranch, duration=self.snoopTime)
    checkValues = {}
    column_evtno = 8
    if len(cpuList) == 1:
      checkValues = {'0x00a1': '>990', '0x00a2': '1', '0x00a3': '1'}
    if len(cpuList) == 2:
      checkValues.update({'0x00b1': '>990', '0x00b2': '1', '0x00b3': '1'})
    if len(cpuList) == 3:
      checkValues.update({'0x00c1': '>990', '0x00c2': '1', '0x00c3': '1'})
    if len(cpuList) == 4:
      checkValues.update({'0x00d1': '>990', '0x00d2': '1', '0x00d3': '1'})
    self.analyseFrequencyFromCsv(fileName, column_evtno, checkValues=checkValues)
    self.deleteFile(fileName)
    self.deleteFile(self.schedulesFolder + scheduleFile)

  def test_branchCPU0(self):
    self.run_test_branch(['A'])

  def test_branchCPU01(self):
    self.run_test_branch(['A', 'B'])

  def test_branchCPU012(self):
    self.run_test_branch(['A', 'B', 'C'])

  def test_branchCPU0123(self):
    self.run_test_branch(['A', 'B', 'C', 'D'])
