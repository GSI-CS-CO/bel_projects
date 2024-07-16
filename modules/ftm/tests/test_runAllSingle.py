import dm_testbench

"""Class collects unit tests to run a pattern on a single cpu.
This is tested for all four cpus.
"""
class UnitTestRunAllSingle(dm_testbench.DmTestbench):

  def common_dynamic_run_all_single(self, number):
    """Load the schedule dynamic-basic-run_all_single-schedule.dot.
    Check that no node is visited. Then start pattern IN_C?.
    Check again the visited nodes.
    """
    scheduleFile = 'dynamic-basic-run_all_single-schedule.dot'
    expectedFile1 = 'dynamic-basic-run_all_single-expected-0-0.txt'
    expectedFile2 = f'dynamic-basic-run_all_single-expected-{number}-2.txt'
    self.generateSchedule(self.schedulesFolder + scheduleFile, number)
    self.addSchedule(scheduleFile)
    self.deleteFile(self.schedulesFolder + scheduleFile)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.generateExpected(self.schedulesFolder + expectedFile1, number)
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + expectedFile1)
    self.deleteFile(self.schedulesFolder + expectedFile1)
    if number < self.cpuQuantity:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_C' + str(number)))
    self.delay(0.1)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.generateExpected(self.schedulesFolder + expectedFile2, number, True)
    self.compareExpectedOutput(stdoutLines, self.schedulesFolder + expectedFile2)
    self.deleteFile(self.schedulesFolder + expectedFile2)

  def test_dynamic_run_cpu0(self):
    self.common_dynamic_run_all_single(0)

  def test_dynamic_run_cpu1(self):
    self.common_dynamic_run_all_single(1)

  def test_dynamic_run_cpu2(self):
    self.common_dynamic_run_all_single(2)

  def test_dynamic_run_cpu3(self):
    self.common_dynamic_run_all_single(3)

  def generateSchedule(self, scheduleFile, cpus):
    """Generate the following schedule and write it to a file. For each
    CPU generate a block.
digraph g {
name="BasicRunAllControlTest";
edge  [type="defdst"]
node  [cpu="0"];

  BLOCK_IN_C0  [cpu="0", type="block", pattern="IN_C0", patentry="true",  patexit="true", qil="1", qhi="1", qlo="1", tperiod=100000000];
  BLOCK_IN_C1  [cpu="1", type="block", pattern="IN_C1", patentry="true",  patexit="true", qil="1", qhi="1", qlo="1", tperiod=100000000];
  BLOCK_IN_C2  [cpu="2", type="block", pattern="IN_C2", patentry="true",  patexit="true", qil="1", qhi="1", qlo="1", tperiod=100000000];
  BLOCK_IN_C3  [cpu="3", type="block", pattern="IN_C3", patentry="true",  patexit="true", qil="1", qhi="1", qlo="1", tperiod=100000000];

}
          """
    lines = []
    lines.append(f'digraph BasicRunAllControlTest ' + '{')
    lines.append(f'  name=BasicRunAllControlTest')
    lines.append(f'  node [type=block patentry=1 patexit=1 qil=1 qhi=1 qlo=1 tperiod=100000000]')
    print(f'{cpus=}, {self.cpuQuantity=}, {min(cpus + 1, self.cpuQuantity)=}')
    for i in range(min(cpus + 1, self.cpuQuantity)):
      lines.append(f'  BLOCK_IN_C{i:d} [cpu={i} pattern=IN_C{i}]')
    lines.append('}')
    lines.append('')
    # write the file
    with open(scheduleFile, 'w') as file1:
      file1.write("\n".join(lines))

  def generateExpected(self, expectedFile, number, generateSecond=False):
    """
      BLOCK_IN_C0:0
      BLOCK_IN_C1:0
      BLOCK_IN_C2:0
      BLOCK_IN_C3:0
        """
    lines = []
    cpus = min(number + 1, self.cpuQuantity)
    for i in range(cpus):
      if generateSecond and i == number:
        lines.append(f'BLOCK_IN_C{i:d}:1')
      else:
        lines.append(f'BLOCK_IN_C{i:d}:0')
    lines.append('')
    # write the file
    with open(expectedFile, 'w') as file1:
      file1.write("\n".join(lines))
