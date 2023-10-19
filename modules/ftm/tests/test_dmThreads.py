import dm_testbench
import pytest

"""
Class starts dmThread tests and compares with expected result.
"""
class UnitTestDatamasterThreads(dm_testbench.DmTestbench):

  def run_dmThreads(self, count):
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'reset', 'all'), [0])
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'clear', '-f'), [0])
    self.resetAllCpus()
    scheduleFile = f'pps-all-threads-cpu0-{count}.dot'
    self.generate_schedule(scheduleFile, count)
    self.addSchedule(f'../{scheduleFile}')
    self.assertIn(count, range(1,len(self.patternNames)+1), f'Number of threads is {count}, not in {range(1,len(self.patternNames)+1)}')
    for i in range(count):
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'Pattern0_' + chr(self.patternNames[i]), '-t', str(i)), [0])
    self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster), [0], 13 + count)
    self.checkRunningThreadsCmd()
    self.deleteFile(scheduleFile)

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

  @pytest.mark.thread32
  def test_dmThreads9(self):
    self.run_dmThreads(9)

  @pytest.mark.thread32
  def test_dmThreads10(self):
    self.run_dmThreads(10)

  @pytest.mark.thread32
  def test_dmThreads11(self):
    self.run_dmThreads(11)

  @pytest.mark.thread32
  def test_dmThreads12(self):
    self.run_dmThreads(12)

  @pytest.mark.thread32
  def test_dmThreads13(self):
    self.run_dmThreads(13)

  @pytest.mark.thread32
  def test_dmThreads14(self):
    self.run_dmThreads(14)

  @pytest.mark.thread32
  def test_dmThreads15(self):
    self.run_dmThreads(15)

  @pytest.mark.thread32
  def test_dmThreads16(self):
    self.run_dmThreads(16)

  @pytest.mark.thread32
  def test_dmThreads17(self):
    self.run_dmThreads(17)

  @pytest.mark.thread32
  def test_dmThreads18(self):
    self.run_dmThreads(18)

  @pytest.mark.thread32
  def test_dmThreads19(self):
    self.run_dmThreads(19)

  @pytest.mark.thread32
  def test_dmThreads20(self):
    self.run_dmThreads(20)

  @pytest.mark.thread32
  def test_dmThreads21(self):
    self.run_dmThreads(21)

  @pytest.mark.thread32
  def test_dmThreads22(self):
    self.run_dmThreads(22)

  @pytest.mark.thread32
  def test_dmThreads23(self):
    self.run_dmThreads(23)

  @pytest.mark.thread32
  def test_dmThreads24(self):
    self.run_dmThreads(24)

  @pytest.mark.thread32
  def test_dmThreads25(self):
    self.run_dmThreads(25)

  @pytest.mark.thread32
  def test_dmThreads26(self):
    self.run_dmThreads(26)

  @pytest.mark.thread32
  def test_dmThreads27(self):
    self.run_dmThreads(27)

  @pytest.mark.thread32
  def test_dmThreads28(self):
    self.run_dmThreads(28)

  @pytest.mark.thread32
  def test_dmThreads29(self):
    self.run_dmThreads(29)

  @pytest.mark.thread32
  def test_dmThreads30(self):
    self.run_dmThreads(30)

  @pytest.mark.thread32
  def test_dmThreads31(self):
    self.run_dmThreads(31)

  @pytest.mark.thread32
  def test_dmThreads32(self):
    self.run_dmThreads(32)

  def test_dmAllThreads_Cpu0123(self):
    count = 8
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'reset', 'all'), [0])
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'clear', '-f'), [0])
    self.resetAllCpus()
    self.addSchedule('pps-all-threads-cpu0.dot')
    self.addSchedule('pps-all-threads-cpu1.dot')
    self.addSchedule('pps-all-threads-cpu2.dot')
    self.addSchedule('pps-all-threads-cpu3.dot')
    index = 0
    threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3'), ('e', '4'), ('f', '5'), ('g', '6'), ('h', '7')]
    for x, y in threadList:
      if index < count:
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS0' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS1' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS2' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS3' + x, '-t', y), [0])
        index = index + 1
    self.checkRunningThreadsCmd()

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
