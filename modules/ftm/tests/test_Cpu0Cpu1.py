import dm_testbench

"""Test a bunch of cases where an edge connects nodes on two CPUs.
"""
class ConnectCpus(dm_testbench.DmTestbench):

  def testTwoCpusFlow(self):
    """Load a schedule with a target edge that connects a node on CPU 0
    with a node on CPU 1. There is also a flowdst edge from CPU 0 to CPU 1.
    """
    snoopFile = 'snoop_TwoCpusFlow.csv'
    self.scheduleFile0 = 'cpu0-1-flow-block-target.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionTwoCpusFlow, duration=1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000000': '>35', '0x0000000000000001': '>35', '0x0000000000000002': '>35'})
    self.deleteFile(snoopFile)

  def actionTwoCpusFlow(self):
    """During snoop start pattern X and A. This produces messages.
    Download the schedule for later compare.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'X'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)

  def testTwoCpusSwitch(self):
    """Load a schedule with a target edge that connects a node on CPU 0
    with a node on CPU 1. There is also a switchdst edge from CPU 0 to CPU 1.
    """
    snoopFile = 'snoop_TwoCpusSwitch.csv'
    self.scheduleFile0 = 'cpu0-1-switch-block-target.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionTwoCpusSwitch, duration=1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000000': '>35', '0x0000000000000001': '>0', '0x0000000000000002': '>35'})
    self.deleteFile(snoopFile)

  def actionTwoCpusSwitch(self):
    """During snoop start pattern X and A. This produces messages.
    Download the schedule for later compare.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'X'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)