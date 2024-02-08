import dm_testbench

"""
Start a pps pattern.

Required: set up of DmTestbench class.
"""
class DmPps(dm_testbench.DmTestbench):

  def test_pps(self):
    snoopFile = 'snoop_pps.csv'
    self.startPattern('pps.dot')
    self.snoopToCsv(snoopFile, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x00d7 and 0x00cd occur.
    self.analyseFrequencyFromCsv(snoopFile, column=8, printTable=True, checkValues={'0x00d7': '>0', '0x00cd': '>0'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x234 and 0x123 occur.
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000234': '>0', '0x0000000000000123': '>0'})
    self.deleteFile(snoopFile)

  def test_ppsAdd(self):
    snoopFile = 'snoop_ppsAdd.csv'
    scheduleFile = 'pps-subgraph.dot'
    downloadFile = 'pps-download.dot'
    self.startPattern(scheduleFile)
    self.snoopToCsv(snoopFile, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x0fff occur.
    self.analyseFrequencyFromCsv(snoopFile, column=8, printTable=True, checkValues={'0x0fff': '>1'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x400 occur.
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000400': '>1'})
    self.deleteFile(snoopFile)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile, downloadFile))
    self.deleteFile(downloadFile)

  def testPpsAdd0(self):
    """Add two schedules. The first schedule contains two nodes and an edge.
    The second adds some edges to the first schedule.
    Start pattern A and use a flow command to trigger messages.
    The status of the schedules is compared against known dot files.
    The messages are snooped and checked.
    """
    snoopFile = 'snoop_test0.csv'
    scheduleFile0 = 'pps-test0-0.dot'
    self.scheduleFile1 = 'pps-test0-1.dot'
    self.downloadFile0 = 'pps-test0-0-download.dot'
    downloadFile1 = 'pps-test0-1-download.dot'
    self.addSchedule(scheduleFile0)
    self.snoopToCsvWithAction(snoopFile, self.actionPpsAdd0, duration=1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + downloadFile1, downloadFile1))
    self.deleteFile(downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=8, printTable=True, checkValues={'0x0fff': '>10'})
    self.deleteFile(snoopFile)

  def actionPpsAdd0(self):
    """During snoop start pattern A. This produces 1 message. Pattern A finishes.
    Download the schedule for later compare.
    Add a schedule which contains two edges.
    Queue a flow command with quantity 10 to block B_A.
    Again start pattern A. The flow command triggers the next messages.
    At the end, 12 messages are produced and the pattern loops in block B_A.
    """
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.addSchedule(self.scheduleFile1)
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'flow', '-q', '10', 'B_A', 'Evt_A'])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)

  def testPpsAdd1(self):
    """Add two schedules. The first schedule contains pattern A with two nodes and an edge.
    The second adds a similar pattern B.
    The messages are snooped and checked.
    """
    snoopFile = 'snoop_test1.csv'
    scheduleFile0 = 'pps-test1-0.dot'
    self.scheduleFile1 = 'pps-test1-1.dot'
    self.downloadFile0 = 'pps-test1-0-download.dot'
    downloadFile1 = 'pps-test1-1-download.dot'
    self.addSchedule(scheduleFile0)
    self.snoopToCsvWithAction(snoopFile, self.actionPpsAdd1, duration=1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + downloadFile1, downloadFile1))
    self.deleteFile(downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x000000000000000f': '1', '0x00000000000000f0': '1'})
    self.deleteFile(snoopFile)

  def actionPpsAdd1(self):
    """During snoop start pattern A. This produces 1 message. Pattern A finishes.
    Download the schedule for later compare.
    Add a schedule with pattern B which is similar to pattern B.
    Start pattern B.
    """
    # remote execution: small delay for snoop to start before the pattern is started.
    self.delay(0.1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.addSchedule(self.scheduleFile1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'B'), [0], 1, 0)

  def testPpsAdd2(self):
    """Test that the validation for nodes connected by defdst or altdst on
    the same CPU works. Add a first schedule. Try to add a second schedule.
    This fails due to an edge from CPU 0 to CPU 1.
    Add a third schedule with a target edge from CPU 0 to CPU 1. This works.
    """
    snoopFile0 = 'snoop_test2_0.csv'
    # this is a pps-pattern with 10Hz. Pattern A
    scheduleFile0 = 'pps-test2-0.dot'
    # this is a pattern with an altdst edge from CPU 0 to CPU 1.
    scheduleFile1 = 'pps-test2-1.dot'
    # this is a pattern with a target edge from CPU 0 to CPU 1.
    scheduleFile2 = 'pps-test2-2.dot'
    self.downloadFile0 = 'pps-test2-0-download.dot'
    self.downloadFile1 = 'pps-test2-1-download.dot'
    self.downloadFile2 = 'pps-test2-2-download.dot'
    self.addSchedule(scheduleFile0)
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd2, duration=1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + scheduleFile1], [250], 2, 2)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.addSchedule(scheduleFile2)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2))
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile0, column=8, printTable=True, checkValues={'0x0fff': '>8'})
    self.deleteFile(snoopFile0)

  def actionPpsAdd2(self):
    """During snoop start pattern A. This produces messages at 10Hz.
    Download the schedule for later compare.
    """
    # remote execution: small delay for snoop to start before the pattern is started.
    self.delay(0.1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])

  def testPpsAdd3(self):
    """Test with five schedules.
    Add two schedules, remove the third, add the fourth, remove the fifth.
    """
    snoopFile0 = 'snoop_test3_0.csv'
    self.scheduleFile0 = 'pps-test3-0.dot'
    self.scheduleFile1 = 'pps-test3-1.dot'
    self.scheduleFile2 = 'pps-test3-2.dot'
    self.scheduleFile3 = 'pps-test3-3.dot'
    self.scheduleFile4 = 'pps-test3-4.dot'
    self.downloadFile0 = 'pps-test3-0-download.dot'
    self.downloadFile1 = 'pps-test3-1-download.dot'
    self.downloadFile2 = 'pps-test3-2-download.dot'
    self.downloadFile3 = 'pps-test3-3-download.dot'
    self.downloadFile4 = 'pps-test3-4-download.dot'
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd3, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2))
    self.deleteFile(self.downloadFile2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile3, self.downloadFile3))
    self.deleteFile(self.downloadFile3)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile4, self.downloadFile4))
    self.deleteFile(self.downloadFile4)
    self.analyseFrequencyFromCsv(snoopFile0, column=20, printTable=True, checkValues={
        '0x0000000000000001': '>10', '0x0000000000000002': '>10', '0x0000000000000020': '1', '0x0000000000000021': '1', '0x0000000000000022': '1'})
    self.analyseFrequencyFromCsv(snoopFile0, column=8, printTable=True, checkValues={
        '0x0000': '>10', '0x000f': '>10', '0x00ff': '1', '0x03ff': '1', '0x0fff': '1'})
    self.deleteFile(snoopFile0)

  def actionPpsAdd3(self):
    """During snoop start pattern A. This produces messages at 10Hz.
    Start the other pattern B, C, D. Each produces one message.
    Pattern B is removed with scheduleFile2.
    Download the four schedules for later compare.
    """
    # remote execution: small delay for snoop to start before the pattern is started.
    self.delay(0.1)
    self.startPattern(self.scheduleFile0, 'A')
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startPattern(self.scheduleFile1, 'B')
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'startpattern', 'C'])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.delay(1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile2])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])
    self.startPattern(self.scheduleFile3, 'D')
    self.delay(1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile3])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile4])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile4])

  def testPpsAdd4(self):
    """Test removing a pattern which is running. This is rejected as expected.
    Then stop the pattern A and remove it.
    Check that the appropriate number of messages is produced.
    """
    snoopFile0 = 'snoop_test4_0.csv'
    self.scheduleFile0 = 'pps-test4-0.dot'
    self.scheduleFile1 = 'pps-test4-1.dot'
    self.scheduleFile2 = 'pps-test4-2.dot'
    self.downloadFile0 = 'pps-test4-0-download.dot'
    self.downloadFile1 = 'pps-test4-1-download.dot'
    self.downloadFile2 = 'pps-test4-2-download.dot'
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd4, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2))
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile0, column=20, printTable=True, checkValues={
        '0x0000000000000001': '>10', '0x0000000000000002': '>10'})
    self.analyseFrequencyFromCsv(snoopFile0, column=8, printTable=True, checkValues={
        '0x000f': '>10', '0x00ff': '>10'})
    self.deleteFile(snoopFile0)

  def actionPpsAdd4(self):
    """During snoop start pattern A. This produces messages at 10Hz.
    Start the other pattern B, C, D. Each produces one message.
    Pattern B is removed with self.scheduleFile2.
    Download the four schedules for later compare.
    """
    # remote execution: small delay for snoop to start before the pattern is started.
    self.delay(0.1)
    self.startPattern(self.scheduleFile0, 'A')
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], expectedReturnCode=[250])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.delay(2.0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'))
    self.delay(0.5)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile2], expectedReturnCode=[0])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])

  def testPpsAdd5(self):
    scheduleFile0 = 'pps-test5-0.dot'
    scheduleFile1 = 'pps-test5-1.dot'
    downloadFile0 = 'pps-test5-0-download.dot'
    downloadFile1 = 'pps-test5-1-download.dot'
    self.startPattern(scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile1], expectedReturnCode=[0])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + downloadFile1, downloadFile1))
    self.deleteFile(downloadFile1)
