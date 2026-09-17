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
    # start pattern A
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    # check that thread 0 has 1 message, threads 1,2,3 are running.
    # check message: done in main method with self.analyseFrequencyFromCsv, key 0x0000000000000000.
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
        checkValues={'0x0000000000000000': '1', '0x0000000000000001': '>17', '0x0000000000000002': '>35', '0x0000000000000003': '>26', '0x0000000000000004': '1'})
    self.deleteFile(fileName)

  def test_nodeInTwoThreads(self):
    """Schedule demonstrates that a node can exist in two threads.
    Tmsg1 ist in thread 1, Tmsg2 is in thread 2. Successor for both is Tmsg3.
    Thus we have a Tmsg3 for each Tmsg1 and Tmsg2.
    The loop in thread 0 (nodes Tmsg0, OriginN, StartthreadN, Block0) starts
    the threads 1 and 2 every 10ms. Thread 1 and 2 end with Block3.
    """
    self.startPattern('nodeInTwoThreads.dot', 'A')
    fileName = 'snoop_nodeInTwoThreads.csv'
    self.delay(0.3)
    self.snoopToCsv(fileName, duration=1)
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True,
        checkValues={'0x0000000000000000': '>99', '0x0000000000000001': '100', '0x0000000000000002': '100', '0x0000000000000003': '100', '0x0000000000000003!conflict': '100'})
    self.deleteFile(fileName)
