import dm_testbench
import pytest

"""Class tests the memory limit for CPU 0.

Structure of test:
add the schedule, add a second schedule: OK. When adding a third schedule,
return code 250 comes back. This is expected. The add opration is rolled back.
"""
class UnitTestMemory(dm_testbench.DmTestbench):

  def test_memory_cpu0(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu0.dot'), [250])

  def test_memory_cpu1(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu1.dot'), [250])

  def test_memory_cpu2(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu2.dot'), [250])

  def test_memory_cpu3(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu3.dot'), [250])
