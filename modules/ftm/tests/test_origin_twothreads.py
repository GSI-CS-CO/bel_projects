import dm_testbench

"""
Tests for node types origin and startthread.
"""
class TestOriginTwoThreads(dm_testbench.DmTestbench):

  def startPatternAndSee(self):
    """
    """
    self.delay(0.2)
    # ~ now = datetime.now().time() # time object
    # ~ print("start of startStopPattern ", now)
    # ~ line1 = self.startAndGetSubprocessOutput(('ssh', 'root@fel0069.acc', 'eb-mon', '-d', 'dev/wbm1'), [0], 1, 0)
    # ~ print(f'{line1}')
    # start pattern B
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'B'), [0], 1, 0)
    # check that thread 1 is running
    self.analyseDmCmdOutput('01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])

  def test_twoThreads(self):
    """Thread 0
    """
    self.addSchedule('twothreads.dot')
    fileName = 'snoop_twothreads.csv'
    # ~ print("before snoop ", datetime.now().time())
    self.snoopToCsvWithAction(fileName, self.startPatternAndSee, duration=1)
    # analyse column 20 which contains the parameter.
    # check par=1:>15, par=2:>0 for snoop of 1 second.
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True,
        checkValues={'0x0000000000000001': '>15', '0x0000000000000002': '>0'})
    self.deleteFile(fileName)
