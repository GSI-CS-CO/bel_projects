import dm_testbench

"""Class collects unit tests for loop.
"""
class UnitTestStartStopAbort(dm_testbench.DmTestbench):

  def testDynamicStartAbort(self):
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
    self.generateExpected(fileName1)
    self.generateExpected(fileName3)
    self.generateExpected(fileName5)
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
    self.deleteFile(fileName1)
    self.deleteFile(fileName3)
    self.deleteFile(fileName5)


  def testDynamicStartStop(self):
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
    self.generateExpected(fileName01)
    self.generateExpected(fileName03)
    self.generateExpected(fileName15)
    self.generateExpected(fileName16)
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
    self.deleteFile(fileName01)
    self.deleteFile(fileName03)
    self.deleteFile(fileName15)
    self.deleteFile(fileName16)

  def generateExpected(self, fileName):
    """
CPU:03,THR:29,RUN:0
MSG:000000000
PAT:undefined,NOD:idle, Origin:undefined, OriginPattern:idle

if '-0-3' in fileName:
-CPU:00,THR:00,RUN:1
-MSG:000000001
-PAT:IN_C0,NOD:BLOCK_IN_C0_EX, Origin:IN_C0, OriginPattern:MSG_IN_C0_EN
    """
    lines = []
    for i in range(self.cpuQuantity):
      for j in range(self.threadQuantity):
        if ('-0-3' in fileName or '-1-5' in fileName) and i == 0 and j == 0:
          lines.append(f'CPU:00,THR:00,RUN:1')
          lines.append(f'MSG:000000001')
          lines.append(f'PAT:IN_C0,NOD:BLOCK_IN_C0_EX, Origin:IN_C0, OriginPattern:MSG_IN_C0_EN')
        elif '-0-5' in fileName and i == 0 and j == 0:
          lines.append(f'CPU:00,THR:00,RUN:0')
          lines.append(f'MSG:000000001')
          lines.append(f'PAT:IN_C0,NOD:BLOCK_IN_C0_EX, Origin:IN_C0, OriginPattern:MSG_IN_C0_EN')
        elif '-1-6' in fileName and i == 0 and j == 0:
          lines.append(f'CPU:00,THR:00,RUN:0')
          lines.append(f'MSG:000000001')
          lines.append(f'PAT:undefined,NOD:idle, Origin:IN_C0, OriginPattern:MSG_IN_C0_EN')
        else:
          lines.append(f'CPU:{i:02d},THR:{j:02d},RUN:0')
          lines.append('MSG:000000000')
          lines.append('PAT:undefined,NOD:idle, Origin:undefined, OriginPattern:idle')
    lines.append('')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write('\n'.join(lines))
