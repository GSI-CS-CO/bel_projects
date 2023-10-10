import subprocess
import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the commands preptime, starttime, deadline.
"""
class AbortTests(dm_testbench.DmTestbench):

  threadQuantity = 8
  cpuQuantity = 4

  def testAbortSingleThreadDecimal(self):
    """Loop for all threads setting and reading the preptime.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread, 'abort')

  def testAbortRunningThreads(self):
    """Load a schedule and start all threads. Check that these are running.
    Abort some threads. Check that these are not running.
    """
    cpu = '0x3'
    thread = '0xaa'
    command = 'abort'
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', 'running'), [0], cpuCount, 0)
    if len(lines[0]) > 0:
      print(f'{chr(10).join(lines[0])}')
    if len(lines[1]) > 0:
      print(f'{chr(10).join(lines[1])}')
    for i in range(cpuCount):
      self.assertEqual(lines[0][i], f'CPU {i} Running Threads: 0x0', 'wrong output')
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', command), [0], threadCount * cpuCount, 0)
    if len(lines[0]) > 0:
      print(f'{chr(10).join(lines[0])}')
    if len(lines[1]) > 0:
      print(f'{chr(10).join(lines[1])}')
    threads = self.listFromBits(thread, self.threadQuantity)
    cpus = self.listFromBits(cpu, self.cpuQuantity)
    for i in range(cpuCount):
      for j in range(threadCount):
        self.assertEqual(lines[0][i*threadCount+j], f'CPU {cpus[i]} Thread {threads[j]} aborted.', 'wrong output')
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', 'running'), [0], cpuCount, 0)
    if len(lines[0]) > 0:
      print(f'{chr(10).join(lines[0])}')
    if len(lines[1]) > 0:
      print(f'{chr(10).join(lines[1])}')
    for i in range(cpuCount):
      self.assertEqual(lines[0][i], f'CPU {i} Running Threads: 0x0', 'wrong output')

  def runThreadXCommand(self, cpu, thread, command, assertText=''):
    """Test for one thread. If commandSet=True set the time (parameter) with the command.
    In all cases, read this value. Check the output of both commands.
    """
    threads = self.listFromBits(thread, self.threadQuantity)
    # ~ threadCount = self.threadCount(2**threadD)
    # ~ print(f'{thread=}, {type(thread)=}, {threadCount=}, {threads=}')
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', command), [0], 1, 0)

  def tearDown(self):
    # reset all CPUs to get a clean state. This is not done by dm-cmd reset all.
    self.resetAllCpus()

