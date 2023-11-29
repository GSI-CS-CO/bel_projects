import dm_testbench
# ~ from datetime import datetime

"""
Tests for node types origin and startthread.
"""
class TestOriginStartthread(dm_testbench.DmTestbench):

  def startStopPattern(self):
    """
    """
    self.delay(0.2)
    # ~ now = datetime.now().time() # time object
    # ~ print("start of startStopPattern ", now)
    # ~ line1 = self.startAndGetSubprocessOutput(('ssh', 'root@fel0069.acc', 'eb-mon', '-d', 'dev/wbm1'), [0], 1, 0)
    # ~ print(f'{line1}')
    # start pattern A
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    # check that thread 0 has 1 message, threads 1,2,3 are running
    # TODO check messages
    self.analyseDmCmdOutput(0x0E)
    # start pattern B
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'B'), [0], 1, 0)
    # check that thread 0 has 2 messages, threads 1,2,3 are running
    # TODO check messages
    self.analyseDmCmdOutput(0x0E)
    # stop pattern A1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A1'), [0], 0, 0)
    # check that thread 1 has stopped
    self.analyseDmCmdOutput(0x0C)
    # stop pattern A
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'), [0], 0, 0)
    # check that thread 3 has stopped
    self.analyseDmCmdOutput(0x04)
    # stop block2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stop', 'Block2'), [0], 0, 0)
    # check that thread 2 has stopped
    self.analyseDmCmdOutput(0x00)


  def test_threadsStartStop(self):
    """Thread 0 assigns TmsgX and BlockX to thread 1,2,3 and starts these.
    then start pattern B (Tmsg4 and Block4) to show that this does not stop the other threads.
    Stop pattern A1 which stops thread 1
    Stop pattern A which stops thread 3
    Stop node Block2 which stops thread 2 (there is no pattern on this thread)
    """
    self.addSchedule('threadsStartStop.dot')
    fileName = 'snoop_threadsStartStop.csv'
    # ~ print("before snoop ", datetime.now().time())
    self.snoopToCsvWithAction(fileName, self.startStopPattern, duration=1)
    # analyse column 20 which contains the parameter.
    # check par=0:1, par=1:18, par=2:36, par=3:27, par=4:1 for snoop of 1 second.
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True,
        checkValues={'0x0000000000000000': '1', '0x0000000000000001': '>17', '0x0000000000000002': '>35', '0x0000000000000003': '>26', '0x0000000000000004': '1'})
    self.deleteFile(fileName)

  def test_nodeInTwoThreads(self):
    """Schedule demonstrates that a node can exist in two threads.
    Tmsg1 ist in thread 1, Tmsg2 is in thread 2. Successor for both is Tmsg3.
    Thus we have a Tmsg3 for each Tmsg1 and Tmsg2.
    The loop in thread 0 (nodes Tmsg0, OriginN, StartthreadN, Block0) starts
    the threads 1, 2 every 10ms. Thread 1, 2 end with Block3.
    """
    self.startPattern('nodeInTwoThreads.dot', 'A')
    fileName = 'snoop_nodeInTwoThreads.csv'
    self.delay(0.1)
    self.snoopToCsv(fileName, duration=1)
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True,
        checkValues={'0x0000000000000000': '>99', '0x0000000000000001': '100', '0x0000000000000002': '100', '0x0000000000000003': '100', '0x0000000000000003!conflict': '100'})
    self.deleteFile(fileName)

  def test_startStopAllThreads(self):
    """Run a pps pattern on all threads and all CPUs. Check this state.
    Then start four threads 0,1,2,3 on all CPUs and check.
    For more than 8 threads:
    Then start four threads 8,9,10,11 on all CPUs and check.
    Then start four threads 16,17,18,19 on all CPUs and check.
    Then start four threads 24,25,26,27 on all CPUs and check.
    """
    self.prepareRunThreads();
    # ~ self.delay(2.0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'halt'), [0], 0, 0)
    # ~ self.delay(2.0)
    # Check all CPUs that no thread is running.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
    for cpu in range(self.cpuQuantity):
      expectedText = 'CPU {variable} Running Threads: 0x0'.format(variable=cpu)
      self.assertEqual(lines[cpu], expectedText, '0 wrong output, expected: ' + expectedText)
    # Start some threads on all CPUs: 0xf.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', '-t', '0xf', 'start'), [0], 0, 0)
    # Check all CPUs that threads 0,1,2,3 are running.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
    for cpu in range(self.cpuQuantity):
      expectedText = 'CPU {variable} Running Threads: 0xf'.format(variable=cpu)
      self.assertEqual(lines[cpu], expectedText, '1 wrong output, expected: ' + expectedText)
    if self.threadQuantity > 8:
      # Start some threads on all CPUs: 0xf00.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', '-t', '0xf00', 'start'), [0], 0, 0)
      # Check all CPUs that threads 0,1,2,3, 8,9,10,11 are running.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
      for cpu in range(self.cpuQuantity):
        expectedText = 'CPU {variable} Running Threads: 0xf0f'.format(variable=cpu)
        self.assertEqual(lines[cpu], expectedText, '2 wrong output, expected: ' + expectedText)
      # Start some threads on all CPUs: 0xf0000.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', '-t', '0xf0000', 'start'), [0], 0, 0)
      # Check all CPUs that threads 0,1,2,3, 8,9,10,11, 16,17,18,19 are running.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
      for cpu in range(self.cpuQuantity):
        expectedText = 'CPU {variable} Running Threads: 0xf0f0f'.format(variable=cpu)
        self.assertEqual(lines[cpu], expectedText, '3 wrong output, expected: ' + expectedText)
      # Start some threads on all CPUs: 0xf000000.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', '-t', '0xf000000', 'start'), [0], 0, 0)
      # Check all CPUs that threads 0,1,2,3, 8,9,10,11, 16,17,18,19, 24,25,26,27 are running.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0xf', 'running'), [0], self.cpuQuantity, 0)
      for cpu in range(self.cpuQuantity):
        expectedText = 'CPU {variable} Running Threads: 0xf0f0f0f'.format(variable=cpu)
        self.assertEqual(lines[cpu], expectedText, '4 wrong output, expected: ' + expectedText)
