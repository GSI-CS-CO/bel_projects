import dm_testbench

"""
Class starts dmThread tests and compares with expected result.
"""
class UnitTestDatamasterThreads(dm_testbench.DmTestbench):

  def run_dmThreads(self, count):
    if count <= self.threadQuantity:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'reset', 'all'), [0])
      self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'clear', '-f'), [0])
      self.resetAllCpus()
      scheduleFile = f'pps-all-threads-cpu0-{count}.dot'
      self.generate_schedule(scheduleFile, count)
      self.addSchedule(f'../{scheduleFile}')
      self.assertIn(count, range(1,len(self.patternNames)+1), f'Number of threads is {count}, not in {range(1,len(self.patternNames)+1)}')
      for i in range(count):
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'Pattern0_' + chr(self.patternNames[i]), '-t', str(i)), [0])
      self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster), [0], 9 + self.cpuQuantity + count)
      self.checkRunningThreadsCmd()
      self.deleteFile(scheduleFile)
    else:
      self.assertGreater(count, self.threadQuantity)

  def test_dmThreads1(self):
    self.run_dmThreads(1)

  def test_dmThreads2(self):
    self.run_dmThreads(2)

  def test_dmThreads3(self):
    self.run_dmThreads(3)

  def test_dmThreads4(self):
    self.run_dmThreads(4)

  def test_dmThreads5(self):
    self.run_dmThreads(5)

  def test_dmThreads6(self):
    self.run_dmThreads(6)

  def test_dmThreads7(self):
    self.run_dmThreads(7)

  def test_dmThreads8(self):
    self.run_dmThreads(8)

  def test_dmThreads9(self):
    self.run_dmThreads(9)

  def test_dmThreads10(self):
    self.run_dmThreads(10)

  def test_dmThreads11(self):
    self.run_dmThreads(11)

  def test_dmThreads12(self):
    self.run_dmThreads(12)

  def test_dmThreads13(self):
    self.run_dmThreads(13)

  def test_dmThreads14(self):
    self.run_dmThreads(14)

  def test_dmThreads15(self):
    self.run_dmThreads(15)

  def test_dmThreads16(self):
    self.run_dmThreads(16)

  def test_dmThreads17(self):
    self.run_dmThreads(17)

  def test_dmThreads18(self):
    self.run_dmThreads(18)

  def test_dmThreads19(self):
    self.run_dmThreads(19)

  def test_dmThreads20(self):
    self.run_dmThreads(20)

  def test_dmThreads21(self):
    self.run_dmThreads(21)

  def test_dmThreads22(self):
    self.run_dmThreads(22)

  def test_dmThreads23(self):
    self.run_dmThreads(23)

  def test_dmThreads24(self):
    self.run_dmThreads(24)

  def test_dmThreads25(self):
    self.run_dmThreads(25)

  def test_dmThreads26(self):
    self.run_dmThreads(26)

  def test_dmThreads27(self):
    self.run_dmThreads(27)

  def test_dmThreads28(self):
    self.run_dmThreads(28)

  def test_dmThreads29(self):
    self.run_dmThreads(29)

  def test_dmThreads30(self):
    self.run_dmThreads(30)

  def test_dmThreads31(self):
    self.run_dmThreads(31)

  def test_dmThreads32(self):
    self.run_dmThreads(32)

  def test_dmAllThreads_Cpu0123(self):
    self.prepareRunThreads()

  patternNames = list(range(ord('a'), ord('z')+1)) + list(range(ord('A'), ord('Z')+1))

  def generate_schedule(self, fileName, numberOfPatterns, cpu=0):
    """ Generates a schedule with numberOfPatterns pps patterns.

    :param fileName: the file to store the schedule.
    :param numberOfPatterns: number of patterns.
    :param cpu:  CPU for the schedule (Default value = 0)

    edge [type=defdst]
    node [cpu=0 fid=1 toffs=0 tef=0 tperiod=1000000000]
    Block0a[type=block pattern=PPS0a patexit=1]
    Evt0a[type=tmsg pattern=PPS0a patentry=1 gid=10 evtno=210 par=48]
    Block0a->Evt0a->Block0a

    """
    lines = []
    lines.append(f'digraph pps{numberOfPatterns}Patterns {{')
    lines.append(f'edge [type=defdst]')
    lines.append(f'node [cpu={cpu} fid=1 toffs=0 tef=0 tperiod=1000000000]')
    for i in range(numberOfPatterns):
      lines.append(f'Block{cpu}_{chr(self.patternNames[i])} [type=block pattern=Pattern{cpu}_{chr(self.patternNames[i])} patexit=1 qlo=1]')
      lines.append(f'Evt{cpu}_{chr(self.patternNames[i])} [type=tmsg pattern=Pattern{cpu}_{chr(self.patternNames[i])} patentry=1 gid=10 evtno=210 par={str(48+i)}]')
      lines.append(f'Block{cpu}_{chr(self.patternNames[i])} -> Evt{cpu}_{chr(self.patternNames[i])} -> Block{cpu}_{chr(self.patternNames[i])}')
      lines.append('')
    lines.append('}')
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))
