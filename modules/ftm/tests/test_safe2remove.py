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
    self.deleteFile('debug.dot')
    self.deleteFile('status.dot')

  def safe2removeTestcase(self, dot_file1, pattern_to_remove):
    self.startAllPattern(self.datamaster, dot_file1 + '.dot')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'chkrem', pattern_to_remove))
    self.compareExpectedResult('debug.dot', self.schedules_folder + dot_file1 + '-forbidden.dot', 'Created')
    self.deleteFile('debug.dot')
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'abortpattern', pattern_to_remove))
    self.startAndCheckSubprocess(('dm-cmd', self.datamaster, 'chkrem', pattern_to_remove))
    self.compareExpectedResult('debug.dot', self.schedules_folder + dot_file1 + '-safe.dot', 'Created')
    self.startAndCheckSubprocess(('dm-sched', self.datamaster, 'remove', self.schedules_folder + dot_file1 + '-remove.dot'))
    self.startAndCheckSubprocess(('dm-sched', self.datamaster, 'status', '-o', 'status.dot'))
    self.compareExpectedResult('status.dot', self.schedules_folder + dot_file1 + '-status.dot')

  def test_safe2remove_blockalign1(self):
    self.safe2removeTestcase('blockalign1', 'PPS1_TEST')

  def safe2removeTestcasePerformance(self, dot_file1, limit):
    start = dt.now()
    self.safe2removeTestcase(dot_file1, 'G1_P1')
    self.startAndCheckSubprocess(('dm-sched', self.datamaster, 'add', self.schedules_folder + 'g1_p1_update_schedule.dot'))
    duration = dt.now() - start
    self.assertTrue(duration <= limit, f'Duration of test too long, duration: {duration}, limit: {limit}.')

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
    self.safe2removeTestcasePerformance('groups_4_nonDefaultPatterns_9_blocksPerPattern_1', delta(seconds=15))

  @pytest.mark.slow
  def test_safe2remove_group_4_9_10(self):
    self.safe2removeTestcasePerformance('groups_4_nonDefaultPatterns_9_blocksPerPattern_10', delta(seconds=20))

  @pytest.mark.slow
  def test_safe2remove_group_4_9_150(self):
    self.safe2removeTestcasePerformance('groups_4_nonDefaultPatterns_9_blocksPerPattern_150', delta(seconds=95))

  def test_safe2remove_blockflow1(self):
    self.startAllPattern(self.datamaster, 'block-flow1.dot')
    file_name = 'snoop_block-flow1.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 5)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)
