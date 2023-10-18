import subprocess
import dm_testbench    # contains super class
import pytest

"""
Module collects tests for dm-cmd with the command 'clearcpudiag'.
"""
class ClearCpuDiagTests(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    self.threadQuantity = 8
    self.cpuQuantity = 4

  @pytest.mark.thread8
  def testInspectClearCpuDiag(self):
    self.runInspectClearCpuDiagThreads()

  @pytest.mark.thread32
  def testInspectClearCpuDiag32(self):
    self.threadQuantity = 32
    self.runInspectClearCpuDiagThreads()

  def runInspectClearCpuDiagThreads(self):
    """Load a schedule and start all threads. Check that these are running.
    Run clearcpudiag on some threads.
    """
    # Check all CPUs that no thread is running.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
    # ~ self.printStdOutStdErr(lines)
    for i in range(self.cpuQuantity):
      self.assertEqual(lines[0][i], f'CPU {i} Running Threads: 0x0', 'wrong output')
    # Add schedules for all CPUs and start pattern on all threads.
    self.addSchedule('pps-all-threads-cpu0.dot')
    self.addSchedule('pps-all-threads-cpu1.dot')
    self.addSchedule('pps-all-threads-cpu2.dot')
    self.addSchedule('pps-all-threads-cpu3.dot')
    index = 0
    threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3'), ('e', '4'), ('f', '5'), ('g', '6'), ('h', '7'),
                  ('a', '8'), ('b', '9'), ('c', '10'), ('d', '11'), ('e', '12'), ('f', '13'), ('g', '14'), ('h', '15'),
                  ('a', '16'), ('b', '17'), ('c', '18'), ('d', '19'), ('e', '20'), ('f', '21'), ('g', '22'), ('h', '23'),
                  ('a', '24'), ('b', '25'), ('c', '26'), ('d', '27'), ('e', '28'), ('f', '29'), ('g', '30'), ('h', '31')]
    for x, y in threadList:
      if index < self.threadQuantity:
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS0' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS1' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS2' + x, '-t', y), [0])
        self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS3' + x, '-t', y), [0])
        index = index + 1
    self.checkRunningThreadsCmd()
    # Check all CPUs that all threads are running.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
    # ~ self.printStdOutStdErr(lines)
    if self.threadQuantity == 32:
      threadMask = '0xffffffff'
    elif self.threadQuantity == 8:
      threadMask = '0xff'
    else:
      self.assertFalse(True, f'threadQuantity is {self.threadQuantity}, allowed: 8 or 32')
    for i in range(self.cpuQuantity):
      expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
      messageText = 'wrong output, expected: CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
      self.assertEqual(lines[0][i], expectedText, messageText)
    # run clearcpudiag for some CPUs
    cpu = '0x3'
    thread = '0xaa'
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'clearcpudiag'), [0], 0, 0)
    # ~ self.printStdOutStdErr(lines)
