import dm_testbench
import pytest
from datetime import datetime as dt
from datetime import timedelta as delta

"""
Class collects unit tests for safe2remove.
Main test case is:
Clear data master
Add schedule
Start all patterns
Check removal of one pattern, should fail while pattern is running.
Abort pattern
Check removal of this pattern, should be valid, since pattern is not running.
Remove this pattern.
Check status of remaining schedule.
"""
class UnitTestSafe2Remove(dm_testbench.DmTestbench):

  def tearDown(self):
    super().tearDown()
    self.deleteFile('debug.dot')
    self.deleteFile('status.dot')

  def safe2removeTestcase(self, dot_file1, pattern_to_remove, second_pattern=''):
    start = dt.now()
    self.startPattern(dot_file1 + '.dot', pattern_to_remove)
    if len(second_pattern) > 0:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', second_pattern))
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'chkrem', pattern_to_remove))
    duration = dt.now() - start
    self.compareExpectedResult('debug.dot', self.schedulesFolder + dot_file1 + '-forbidden.dot', 'Created')
    self.deleteFile('debug.dot')
    start2 = dt.now()
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'abortpattern', pattern_to_remove))
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'chkrem', pattern_to_remove))
    duration += dt.now() - start2
    self.compareExpectedResult('debug.dot', self.schedulesFolder + dot_file1 + '-safe.dot', 'Created')
    start3 = dt.now()
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + dot_file1 + '-remove.dot'))
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', 'status.dot'))
    duration += dt.now() - start3
    # ~ self.compareExpectedResult('status.dot', self.schedulesFolder + dot_file1 + '-status.dot')
    self.startAndCheckSubprocess(('scheduleCompare', '-s', 'status.dot', self.schedulesFolder + dot_file1 + '-status.dot'), [0], 0, 0)
    return duration

  def test_safe2remove_blockalign1(self):
    self.safe2removeTestcase('blockalign1', 'PPS1_TEST', 'PPS0_TEST')

  def test_safe2remove_blockalign2(self):
    self.safe2removeTestcase('blockalign2', 'A')

  def test_safe2remove_blockalign3(self):
    self.safe2removeTestcase('blockalign3', 'A')

  def safe2removeTestcasePerformance(self, dot_file1, limit):
    start = dt.now()
    duration1 = self.safe2removeTestcase(dot_file1, 'G1_P1')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + 'g1_p1_update_schedule.dot'))
    duration = dt.now() - start
    self.assertGreater(limit, duration1, f'Duration of test too long, duration: {duration1}, limit: {limit}, over all duration: {duration}.')

  def test_safe2remove_group_1_1_1(self):
    self.safe2removeTestcasePerformance('groups_1_nonDefaultPatterns_1_blocksPerPattern_1', delta(seconds=1.1))

  def test_safe2remove_group_1_2_1(self):
    self.safe2removeTestcasePerformance('groups_1_nonDefaultPatterns_2_blocksPerPattern_1', delta(seconds=1.3))

  @pytest.mark.slow
  def test_safe2remove_group_1_4_1(self):
    self.safe2removeTestcasePerformance('groups_1_nonDefaultPatterns_4_blocksPerPattern_1', delta(seconds=1.6))

  @pytest.mark.slow
  def test_safe2remove_group_1_9_1(self):
    self.safe2removeTestcasePerformance('groups_1_nonDefaultPatterns_9_blocksPerPattern_1', delta(seconds=2.8))

  @pytest.mark.slow
  def test_safe2remove_group_1_9_10(self):
    self.safe2removeTestcasePerformance('groups_1_nonDefaultPatterns_9_blocksPerPattern_10', delta(seconds=3.3))

  @pytest.mark.slow
  def test_safe2remove_group_1_9_150(self):
    self.safe2removeTestcasePerformance('groups_1_nonDefaultPatterns_9_blocksPerPattern_150', delta(seconds=12.5))

  @pytest.mark.slow
  def test_safe2remove_group_2_9_1(self):
    self.safe2removeTestcasePerformance('groups_2_nonDefaultPatterns_9_blocksPerPattern_1', delta(seconds=5.5))

  @pytest.mark.slow
  def test_safe2remove_group_2_9_10(self):
    self.safe2removeTestcasePerformance('groups_2_nonDefaultPatterns_9_blocksPerPattern_10', delta(seconds=7))

  @pytest.mark.slow
  def test_safe2remove_group_2_9_150(self):
    self.safe2removeTestcasePerformance('groups_2_nonDefaultPatterns_9_blocksPerPattern_150', delta(seconds=32))

  @pytest.mark.slow
  def test_safe2remove_group_3_9_1(self):
    self.safe2removeTestcasePerformance('groups_3_nonDefaultPatterns_9_blocksPerPattern_1', delta(seconds=9))

  @pytest.mark.slow
  def test_safe2remove_group_3_9_10(self):
    self.safe2removeTestcasePerformance('groups_3_nonDefaultPatterns_9_blocksPerPattern_10', delta(seconds=12))

  @pytest.mark.slow
  def test_safe2remove_group_3_9_150(self):
    self.safe2removeTestcasePerformance('groups_3_nonDefaultPatterns_9_blocksPerPattern_150', delta(seconds=61))

  @pytest.mark.slow
  def test_safe2remove_group_4_9_1(self):
    if self.cpuQuantity > 3:
      self.safe2removeTestcasePerformance('groups_4_nonDefaultPatterns_9_blocksPerPattern_1', delta(seconds=15))

  @pytest.mark.slow
  def test_safe2remove_group_4_9_10(self):
    if self.cpuQuantity > 3:
      self.safe2removeTestcasePerformance('groups_4_nonDefaultPatterns_9_blocksPerPattern_10', delta(seconds=20))

  @pytest.mark.slow
  def test_safe2remove_group_4_9_150(self):
    if self.cpuQuantity > 3:
      self.safe2removeTestcasePerformance('groups_4_nonDefaultPatterns_9_blocksPerPattern_150', delta(seconds=95))

  def test_safe2remove_blockflow1(self):
    self.startAllPattern('block-flow1.dot')
    fileName = 'snoop_block-flow1.csv'
    self.snoopToCsv(fileName, duration=5)
    self.analyseFrequencyFromCsv(fileName, 20, checkValues={'0x00000000000000a1': '>100', '0x00000000000000a2': '>100', '0x00000000000000b1': '>300', '0x00000000000000b2': '>300'})
    self.deleteFile(fileName)
