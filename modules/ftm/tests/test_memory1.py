import dm_testbench
import pytest

"""Class tests the memory limit for CPU 0.

Structure of test:
Generate a schedule with n blocks. Add this to DM. Result schould be
0 for n < 1875 and 250 otherwise.
"""
class UnitTestMemoryFull(dm_testbench.DmTestbench):

  def test_memory_full_bad(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1874)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_bad1(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1870)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_ok(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1869)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_overfull(self):
    fileName = self.schedules_folder + 'memory_full.dot'
    self.generate_schedule(fileName, 1875)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_4cpuOK(self):
    fileName = self.schedules_folder + 'memory_full0.dot'
    self.generate_schedule(fileName, 1869, 0)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full1.dot'
    self.generate_schedule(fileName, 1869, 1)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full2.dot'
    self.generate_schedule(fileName, 1869, 2)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full3.dot'
    self.generate_schedule(fileName, 1675, 3)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_4cpuFail(self):
    fileName = self.schedules_folder + 'memory_full0.dot'
    self.generate_schedule(fileName, 1869, 0)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full1.dot'
    self.generate_schedule(fileName, 1869, 1)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full2.dot'
    self.generate_schedule(fileName, 1869, 2)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedules_folder + 'memory_full3.dot'
    self.generate_schedule(fileName, 1676, 3)
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'add',
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
