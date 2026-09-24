import dm_testbench

"""
Start a pps pattern.

Required: set up of DmTestbench class.
"""
class DmPps(dm_testbench.DmTestbench):

  def testPps0(self):
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

  def testPpsAdd(self):
    snoopFile = 'snoop_ppsAdd.csv'
    self.scheduleFile0 = 'pps-subgraph.dot'
    self.downloadFile0 = 'pps-download.dot'
    self.startPattern(self.scheduleFile0)
    self.snoopToCsv(snoopFile, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x0fff occur.
    self.analyseFrequencyFromCsv(snoopFile, column=8, printTable=True, checkValues={'0x0fff': '>1'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x400 occur.
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000400': '>1'})
    self.deleteFile(snoopFile)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)

  def testPpsAdd0(self):
    """Add two schedules. The first schedule contains two nodes and an edge.
    The second adds some edges to the first schedule.
    Start pattern A and use a flow command to trigger messages.
    The status of the schedules is compared against known dot files.
    The messages are snooped and checked.
    """
    snoopFile = 'snoop_PpsAdd0.csv'
    self.scheduleFile0 = 'pps-test0-0.dot'
    self.scheduleFile1 = 'pps-test0-1.dot'
    self.downloadFile0 = 'pps-test0-0-download.dot'
    self.downloadFile1 = 'pps-test0-1-download.dot'
    self.snoopToCsvWithAction(snoopFile, self.actionPpsAdd0, duration=1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
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
    self.delay(0.1)
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.addSchedule(self.scheduleFile1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'flow', '-q', '10', 'B_A', 'Evt_A'), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)

  def testPpsAdd1(self):
    """Add two schedules. The first schedule contains pattern A with two nodes and an edge.
    The second adds a similar pattern B.
    The messages are snooped and checked.
    """
    snoopFile = 'snoop_PpsAdd1.csv'
    self.scheduleFile0 = 'pps-test1-0.dot'
    self.scheduleFile1 = 'pps-test1-1.dot'
    self.downloadFile0 = 'pps-test1-0-download.dot'
    self.downloadFile1 = 'pps-test1-1-download.dot'
    self.addSchedule(self.scheduleFile0)
    self.snoopToCsvWithAction(snoopFile, self.actionPpsAdd1, duration=1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x000000000000000f': '1', '0x00000000000000f0': '1'})
    self.deleteFile(snoopFile)

  def actionPpsAdd1(self):
    """During snoop start pattern A. This produces 1 message. Pattern A finishes.
    Download the schedule for later compare.
    Add a schedule with pattern B which is similar to pattern B.
    Start pattern B.
    """
    # remote execution: small delay for snoop to start before the pattern is started.
    self.delay(0.5)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.addSchedule(self.scheduleFile1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'B'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)

  def testPpsAdd2(self):
    """Test that the validation for nodes connected by defdst or altdst on
    the same CPU works. Add a first schedule. Try to add a second schedule.
    This fails due to an edge from CPU 0 to CPU 1.
    Add a third schedule with a target edge from CPU 0 to CPU 1. This works.
    """
    snoopFile0 = 'snoop_PpsAdd2.csv'
    # this is a pps-pattern with 10Hz. Pattern A
    self.scheduleFile0 = 'pps-test2-0.dot'
    # this is a pattern with an altdst edge from CPU 0 to CPU 1.
    self.scheduleFile1 = 'pps-test2-1.dot'
    # this is a pattern with a target edge from CPU 0 to CPU 1.
    self.scheduleFile2 = 'pps-test2-2.dot'
    self.downloadFile0 = 'pps-test2-0-download.dot'
    self.downloadFile1 = 'pps-test2-1-download.dot'
    self.downloadFile2 = 'pps-test2-2-download.dot'
    self.addSchedule(self.scheduleFile0)
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd2, duration=1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + self.scheduleFile1), [250], 2, 2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)
    self.addSchedule(self.scheduleFile2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2), [0], 0, 0)
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
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)

  def testPpsAdd3(self):
    """Test with five schedules.
    Add two schedules, remove the third, add the fourth, remove the fifth.
    """
    snoopFile0 = 'snoop_PpsAdd3.csv'
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
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2), [0], 0, 0)
    self.deleteFile(self.downloadFile2)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile3, self.downloadFile3), [0], 0, 0)
    self.deleteFile(self.downloadFile3)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile4, self.downloadFile4), [0], 0, 0)
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
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.delay(0.1)
    self.startPattern(self.scheduleFile1, 'B')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'C'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)
    self.delay(1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile2), [0], 0, 5)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2), [0], 0, 0)
    self.delay(0.1)
    self.startPattern(self.scheduleFile3, 'D')
    self.delay(1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile3), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'), [0], 0, 0)
    self.delay(0.2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile4), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile4), [0], 0, 0)

  def testPpsAdd4(self):
    """Test removing a pattern which is running. This is rejected as expected.
    Then stop the pattern A and remove it.
    Check that the appropriate number of messages is produced.
    """
    snoopFile0 = 'snoop_PpsAdd4.csv'
    self.scheduleFile0 = 'pps-test4-0.dot'
    self.scheduleFile1 = 'pps-test4-1.dot'
    self.scheduleFile2 = 'pps-test4-2.dot'
    self.downloadFile0 = 'pps-test4-0-download.dot'
    self.downloadFile1 = 'pps-test4-1-download.dot'
    self.downloadFile2 = 'pps-test4-2-download.dot'
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd4, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2), [0], 0, 0)
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile0, column=20, printTable=True, checkValues={
        '0x0000000000000001': '>10', '0x0000000000000002': '>10'})
    self.analyseFrequencyFromCsv(snoopFile0, column=8, printTable=True, checkValues={
        '0x000f': '>10', '0x00ff': '>10'})
    self.deleteFile(snoopFile0)

  def actionPpsAdd4(self):
    """During snoop start pattern A. This produces messages at 10Hz.
    Try to remove pattern A while running. This fails. Stop pattern A
    and remove nodes Evt_A and B_A. Check status after this and
    download the four schedules for later compare.
    """
    # remote execution: small delay for snoop to start before the pattern is started.
    self.delay(0.3)
    self.startPattern(self.scheduleFile0, 'A')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1), [250], 2, 2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)
    self.delay(2.0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'))
    self.delay(0.5)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile2), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2), [0], 0, 0)

  def testPpsAdd5(self):
    """Test removing a pattern which is running. This is rejected as expected.
    When pattern A has finished, remove it.
    Check that the appropriate number of messages is produced.
    """
    snoopFile0 = 'snoop_PpsAdd5.csv'
    self.scheduleFile0 = 'pps-test5-0.dot'
    self.scheduleFile1 = 'pps-test5-1.dot'
    self.downloadFile0 = 'pps-test5-0-download.dot'
    self.downloadFile1 = 'pps-test5-1-download.dot'
    self.downloadFile2 = 'pps-test5-2-download.dot'
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd5, duration=1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2), [0], 0, 0)
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile0, column=20, printTable=True, checkValues={
        '0x0000000000000001': '1', '0x0000000000000002': '1'})
    self.analyseFrequencyFromCsv(snoopFile0, column=8, printTable=True, checkValues={
        '0x000f': '1', '0x00ff': '1'})
    self.deleteFile(snoopFile0)

  def actionPpsAdd5(self):
    """During snoop start pattern A. This produces 2 messages.
    While the pattern runs, try to remove part of the schedule.
    This fails (return 250). Later the second try for remove
    works. The status between the steps is saved for later compare.
    """
    self.delay(0.5)
    self.startPattern(self.scheduleFile0, 'A')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1), [250], 2, 22)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)
    self.delay(0.1)
    # pattern has ended, try to remove a second time.
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2), [0], 0, 0)

  def testPpsAdd6(self):
    """Test adding a node which changes the pattern entry.
    Check that the appropriate number of messages is produced.
    """
    snoopFile0 = 'snoop_PpsAdd6.csv'
    self.scheduleFile0 = 'pps-test6-0.dot'
    self.scheduleFile1 = 'pps-test6-1.dot'
    self.scheduleFile2 = 'pps-test6-2.dot'
    self.downloadFile0 = 'pps-test6-0-download.dot'
    self.downloadFile1 = 'pps-test6-1-download.dot'
    self.downloadFile2 = 'pps-test6-2-download.dot'
    self.snoopToCsvWithAction(snoopFile0, self.actionPpsAdd6, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2), [0], 0, 0)
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile0, column=20, printTable=True, checkValues={
        '0x0000000000000001': '>10', '0x0000000000000002': '>8'})
    self.deleteFile(snoopFile0)

  def actionPpsAdd6(self):
    """During snoop start pattern A. Stop pattern after a second.
    Overwrite the schedule such that the edge from BlockA to EvtA changes to altdst.
    Add a schedule with additional node EvtB and start pattern A again.
    The status between the steps is saved for later compare.
    """
    self.delay(0.1)
    self.startPattern(self.scheduleFile0, 'A')
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'), [0], 0, 0)
    self.delay(0.5)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'overwrite', self.schedulesFolder + self.scheduleFile1), [0], 0, 0)
    # this schedule has no defined pattern (entry is missing). Thus, we cannot start a pattern.
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1), [0], 0, 0)
    self.startPattern(self.scheduleFile2, 'A')
    # the additional node EvtB is pattern entry. Run the pattern for a second.
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'), [0], 0, 0)

  def testPpsAdd8(self):
    """Test adding a schedule with a node with name ending in ListDst_3. This fails,
    because this name is generated during upload and a collision happens.
    Second attempt is to add a similar schedule with the name Block0_0_ListDst_6.
    This works. The generated nodes have the names Block0_0_ListDst_x with x from 0 to 5.
    """
    self.scheduleFile0 = 'pps-test8-0.dot'
    self.scheduleFile1 = 'pps-test8-1.dot'
    self.downloadFile1 = 'pps-test8-1-download.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + self.scheduleFile0), [250], 2, 2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + self.scheduleFile1), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-so', self.downloadFile1), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
