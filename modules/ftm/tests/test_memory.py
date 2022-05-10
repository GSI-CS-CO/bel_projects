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
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu0.dot'), [250])

  def test_memory_cpu1(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu1.dot'), [250])

  def test_memory_cpu2(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu2.dot'), [250])

  def test_memory_cpu3(self):
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot')
    self.addSchedule('groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedules_folder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu3.dot'), [250])

"""Class tests the memory limit for CPU 0.

Structure of test:
Generate a schedule with n blocks. Add this to DM. Result schould be
0 for n < 1875 and 250 otherwise.
"""
class UnitTestMemoryFull(dm_testbench.DmTestbench):

  def test_memory_full_bad(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1874)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_bad1(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1870)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_ok(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1869)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_overfull(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1875)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_4cpuOK(self):
    fileName = self.schedules_folder + 'memory_full0.dot'
    self.generate_schedule(fileName, 1869, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full1.dot'
    self.generate_schedule(fileName, 1869, 1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full2.dot'
    self.generate_schedule(fileName, 1869, 2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full3.dot'
    self.generate_schedule(fileName, 1675, 3)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_4cpuFail(self):
    fileName = self.schedules_folder + 'memory_full0.dot'
    self.generate_schedule(fileName, 1869, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full1.dot'
    self.generate_schedule(fileName, 1869, 1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full2.dot'
    self.generate_schedule(fileName, 1869, 2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full3.dot'
    self.generate_schedule(fileName, 1676, 3)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)
    self.deleteFile(fileName)

  def generate_schedule(self, fileName, numberOfBlocks, cpu=0):
    lines = []
    lines.append('digraph memFull {')
    lines.append(f'node [cpu={cpu} type=block tperiod=1000]')
    for i in range(numberOfBlocks):
      lines.append(f'Block{cpu}_{i:04d} [pattern=Pattern{cpu}_{i:04d} patentry=1 patexit=1]')
    lines.append('}')
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))
