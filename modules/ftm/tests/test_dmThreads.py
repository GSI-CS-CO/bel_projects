import dm_testbench
import pytest

"""
Class starts dmThread tests and compares with expected result.
"""
class UnitTestDatamasterThreads(dm_testbench.DmTestbench):

  def run_dmThreads(self, count):
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'reset', 'all'), [0])
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'clear', '-f'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '0'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '1'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '2'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '3'), [0])
    self.delay(1)
    self.addSchedule('../dmThreads/pps-all-threads-cpu0.dot')
    index = 0
    threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3'), ('e', '4'), ('f', '5'), ('g', '6'), ('h', '7')]
    for x, y in threadList:
      if index < count:
        self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'PPS0' + x, '-t', y), [0])
        index = index + 1
    self.startAndGetSubprocessOutput((self.binary_dm_cmd, self.datamaster), [0], 13 + count)
    self.check_dmThreads_Cmd()

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

  def analyse_dm_cmd_output(self, output):
    msgCounts = {}
    firstCount = 0
    index = 0
    countLines = len(output)
    offset = 0
    offset1 = 0
    cpu = 0
    thread = 0
    for line in output:
      if index > 11 and index < (countLines - 1):
        try:
          cpu = int(line[3 + offset1])
          thread = int(line[9 + offset1])
          count = int(line[32 + offset:42 + offset])
        except ValueError:
          print(f'ValueError:{index} CPU {cpu} Thread {thread}  line: "{line}", "{line[3 + offset1]}", "{line[9 + offset1]}", "{line[32 + offset:42 + offset]}", offset={offset}, offset1={offset1}')
        msgCounts[str(10*cpu) + str(thread)] = str(count)
        if offset == 0:
          offset = 10
          offset1 = 5
        else:
          offset = 0
          offset1 = 0
      index = index + 1
    self.assertEqual(countLines-13, len(msgCounts))
    return msgCounts

  def check_dmThreads_Cmd(self):
    output = self.startAndGetSubprocessOutput((self.binary_dm_cmd, self.datamaster), [0])
    firstCounts = self.analyse_dm_cmd_output(output[0])
    self.delay(1)
    output = self.startAndGetSubprocessOutput((self.binary_dm_cmd, self.datamaster), [0])
    secondCounts = self.analyse_dm_cmd_output(output[0])
    for key in firstCounts:
      firstCount = int(firstCounts[key])
      secondCount = int(secondCounts[key])
      # ~ print(f'key={key}, firstCount={firstCount}, secondCount={secondCount}')
      cpu = int(key[0])
      thread = int(key[1])
      self.assertGreater(secondCount, firstCount, f'CPU {cpu} Thread {thread} First: {firstCount}, second: {secondCount}')
      self.assertGreater(firstCount, 0, f'CPU {cpu} Thread {thread} firstCount is {firstCount}')

  def test_dmAllThreads_Cpu0123(self):
    count = 8
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'reset', 'all'), [0])
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'clear', '-f'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '0'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '1'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '2'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '3'), [0])
    self.delay(1)
    self.addSchedule('../dmThreads/pps-all-threads-cpu0.dot')
    self.addSchedule('../dmThreads/pps-all-threads-cpu1.dot')
    self.addSchedule('../dmThreads/pps-all-threads-cpu2.dot')
    self.addSchedule('../dmThreads/pps-all-threads-cpu3.dot')
    index = 0
    threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3'), ('e', '4'), ('f', '5'), ('g', '6'), ('h', '7')]
    for x, y in threadList:
      if index < count:
        self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'PPS0' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'PPS1' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'PPS2' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'PPS3' + x, '-t', y), [0])
        index = index + 1
    self.check_dmThreads_Cmd()
