import dm_testbench
# ~ from datetime import datetime

"""
Tests for node types origin and startthread.
"""
class TestOriginStartthread(dm_testbench.DmTestbench):

  def startStopPattern(self):
    """Start and stop patterns on different threads.
    Check that the correct threads are running-
    """
    self.delay(0.4)
    # ~ now = datetime.now().time() # time object
    # ~ print("start of startStopPattern ", now)
    # ~ line1 = self.startAndGetSubprocessOutput(('ssh', 'root@fel0069.acc', 'eb-mon', '-d', 'dev/wbm1'), [0], 1, 0)
    # ~ print(f'{line1}')
    # start pattern X
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'X'), [0], 1, 0)
    # check that thread 0 has 1 message, threads 1,2,3 are running.
    # check message: done in main method with self.analyseFrequencyFromCsv, key 0x0000000000000000.
    self.analyseDmCmdOutput('01110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])
    # start pattern B
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'D'), [0], 1, 0)
    # check that thread 0 has 2 messages, threads 1,2,3 are running
    # check message: done in main method with self.analyseFrequencyFromCsv, key 0x0000000000000004.
    self.analyseDmCmdOutput('01110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])
    # stop pattern A
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'), [0], 0, 0)
    # check that thread 1 has stopped
    self.analyseDmCmdOutput('00110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])
    # stop pattern A
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'C'), [0], 0, 0)
    # check that thread 3 has stopped
    self.analyseDmCmdOutput('00100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])
    # stop block2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stop', 'Block2'), [0], 0, 0)
    # check that thread 2 has stopped
    self.analyseDmCmdOutput('00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])


  def test_threadsStartStop(self):
    """Thread 0 assigns TmsgX and BlockX to thread X=1,2,3 and starts these threads.
    Then start pattern D (Tmsg4 and Block4 on thread 0) to show that this does not stop the other threads.
    Stop pattern A which stops thread 1.
    Stop pattern C which stops thread 3.
    Stop node Block2 which stops thread 2.
    A good test run has 2 messages on thread 0 (pattern X and pattern D), nearly 20 messages on thread 1 (pattern A),
    nearly 40 messages on thread 2 (pattern B), nearly 30 messages on thread 3 (pattern C). Total of about 90 messages.
    """
    self.addSchedule('threadsStartStop.dot')
    fileName = 'snoop_threadsStartStop.csv'
    # ~ print("before snoop ", datetime.now().time())
    self.snoopToCsvWithAction(fileName, self.startStopPattern, duration=2)
    # analyse column 20 which contains the parameter.
    # check par=0:1, par=1:18, par=2:36, par=3:27, par=4:1 for snoop of 1 second.
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True,
        checkValues={'0x0000000000000000': '1', '0x0000000000000001': '>17', '0x0000000000000002': '>35', '0x0000000000000003': '>26', '0x0000000000000004': '1'}, addDelayed=True)
    self.deleteFile(fileName)

  def test_nodeInTwoThreads(self):
    """Schedule demonstrates that a node can exist in two threads.
    Tmsg1 ist in thread 1, Tmsg2 is in thread 2. Successor for both is Tmsg3.
    Thus we have a Tmsg3 for each Tmsg1 and Tmsg2.
    The loop in thread 0 (nodes Tmsg0, OriginN, StartthreadN, Block0) starts
    the threads 1 and 2 every 10ms. Thread 1 and 2 end with Block3.
    """
    snoopFileName = 'snoop_nodeInTwoThreads.csv'
    self.snoopToCsvWithAction(snoopFileName, self.actionNodeInTwoThreads, duration=2)
    self.analyseFrequencyFromCsv(snoopFileName, column=20, printTable=True,
        checkValues={'0x0000000000000000': '>100', '0x0000000000000001': '>100', '0x0000000000000002': '>100', '0x0000000000000003': '>100', '0x0000000000000003!conflict': '>100'})
    self.deleteFile(snoopFileName)

  def actionNodeInTwoThreads(self):
    self.delay(0.3)
    self.startPattern('nodeInTwoThreads.dot', 'A')
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'), [0], 0, 0)

  def test_startStopAllThreads(self):
    """Run a pps pattern on all threads and all CPUs. Halt all threads and check this state.
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
    allCpuMask = self.maskFromList(range(self.cpuQuantity), self.cpuQuantity)
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, 'running'), [0], self.cpuQuantity, 0)
    for cpu in range(self.cpuQuantity):
      expectedText = 'CPU {variable} Running Threads: 0x0'.format(variable=cpu)
      self.assertEqual(lines[cpu], expectedText, '0 wrong output, expected: ' + expectedText)
    # Start some threads on all CPUs: 0xf.
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, '-t', '0xf', 'start'), [0], 0, 0)
    # Check all CPUs that threads 0,1,2,3 are running.
    lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, 'running'), [0], self.cpuQuantity, 0)
    for cpu in range(self.cpuQuantity):
      expectedText = 'CPU {variable} Running Threads: 0xf'.format(variable=cpu)
      self.assertEqual(lines[cpu], expectedText, '1 wrong output, expected: ' + expectedText)
    if self.threadQuantity > 8:
      # Start some threads on all CPUs: 0xf00.
      self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, '-t', '0xf00', 'start'), [0], 0, 0)
      # Check all CPUs that threads 0,1,2,3, 8,9,10,11 are running.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, 'running'), [0], self.cpuQuantity, 0)
      for cpu in range(self.cpuQuantity):
        expectedText = 'CPU {variable} Running Threads: 0xf0f'.format(variable=cpu)
        self.assertEqual(lines[cpu], expectedText, '2 wrong output, expected: ' + expectedText)
      # Start some threads on all CPUs: 0xf0000.
      self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, '-t', '0xf0000', 'start'), [0], 0, 0)
      # Check all CPUs that threads 0,1,2,3, 8,9,10,11, 16,17,18,19 are running.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, 'running'), [0], self.cpuQuantity, 0)
      for cpu in range(self.cpuQuantity):
        expectedText = 'CPU {variable} Running Threads: 0xf0f0f'.format(variable=cpu)
        self.assertEqual(lines[cpu], expectedText, '3 wrong output, expected: ' + expectedText)
      # Start some threads on all CPUs: 0xf000000.
      self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, '-t', '0xf000000', 'start'), [0], 0, 0)
      # Check all CPUs that threads 0,1,2,3, 8,9,10,11, 16,17,18,19, 24,25,26,27 are running.
      lines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, 'running'), [0], self.cpuQuantity, 0)
      for cpu in range(self.cpuQuantity):
        expectedText = 'CPU {variable} Running Threads: 0xf0f0f0f'.format(variable=cpu)
        self.assertEqual(lines[cpu], expectedText, '4 wrong output, expected: ' + expectedText)

  def test_startStopBlocks(self):
    """Run a pps pattern on all threads of CPU 0.
    Stop block Block0b four times and check the result.
    """
    # Start pps pattern on all threads of CPU 0.
    self.prepareRunThreads(cpus=1);
    lines = ['']
    # Stop one of threads 1, 9, 17, 25. The firmware scheduler decides which thread is choosen.
    self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'stop', 'Block0b'), [0], 0, 0)
    # A delay is needed since the block has to be executed once to process the stop command.
    # Since the block in the pps pattern has a period of 1 second, the delay is 1 second.
    self.delay(1.0)
    lines[0] = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0x1', 'running'), [0], 1, 0)[0]
    if self.threadQuantity > 8:
      self.checkRunningThreads(lines, ['0xfffffffd', '0xfdffffff', '0xfffdffff', '0xfffffdff'])
      self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'stop', 'Block0b'), [0], 0, 0)
      self.delay(1.0)
      lines[0] = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0x1', 'running'), [0], 1, 0)[0]
      self.checkRunningThreads(lines, ['0xfffffdfd', '0xfffdfffd', '0xfdfffffd', '0xfffdfdff', '0xfdfffdff', '0xfdfdffff'])
      self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'stop', 'Block0b'), [0], 0, 0)
      self.delay(1.0)
      lines[0] = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0x1', 'running'), [0], 1, 0)[0]
      self.checkRunningThreads(lines, ['0xfffdfdfd', '0xfdfffdfd', '0xfdfdfffd', '0xfdfdfdff'])
      self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'stop', 'Block0b'), [0], 0, 0)
      self.delay(1.0)
      lines[0] =self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-c', '0x1', 'running'), [0], 1, 0)[0]
      self.checkRunningThreads(lines, ['0xfdfdfdfd'])
    else:
      self.checkRunningThreads(lines, ['0xfd'])
