import dm_testbench

"""Test a bunch of cases for dm-sched remove.
"""
class DmSchedRemove(dm_testbench.DmTestbench):

  def testRemove1(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove the pattern which is nearly the same, but
    the block has no type. The result is a schedule with only the block.
    snoop catches 12 timing messages.
    """
    snoopFile = 'snoop_remove1.csv'
    self.scheduleFile0 = 'remove1-0.dot'
    self.scheduleFile1 = 'remove1-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove1, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove1(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    """
    self.delay(0.3)
    self.startPattern(self.scheduleFile0, 'A')
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testRemove2(self):
    """Load and start pattern A, a loop of a message EvtA and a block BlockA.
    Stop the pattern and remove parts of the pattern which is nearly the same, but
    the block has no type and a different event node EvtC. Remove fails,
    because EvtC is unknown in the existing schedule.
    """
    snoopFile = 'snoop_remove2.csv'
    self.scheduleFile0 = 'remove2-0.dot'
    self.scheduleFile1 = 'remove2-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove2, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove2(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule. This is an
    expected failure.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    lines = self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], [250], 2, 2)
    self.assertEqual(lines[1][1], "../bin/dm-sched: Failed to execute <remove>. Cause:  HashTable: Name EvtC not found")
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testRemove3(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove the pattern which is nearly the same, but
    the block has no type.
    The difference to testRemove1 is that the schedule for removal contains
    EvtA without a type. Thus, the remove command tries to remove BlockA,
    which would leave EvtA childless.
    snoop catches 12 timing messages.
    """
    snoopFile = 'snoop_remove3.csv'
    self.scheduleFile0 = 'remove3-0.dot'
    self.scheduleFile1 = 'remove3-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove3, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove3(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    """
    self.delay(0.3)
    self.startPattern(self.scheduleFile0, 'A')
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    lines = self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], [250], 2, 3)
    self.assertEqual(lines[1][1], "../bin/dm-sched: Failed to execute <remove>. Cause: Validation of Neighbourhood: Node 'EvtA' of type 'tmsg' cannot be childless")
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testRemove4(self):
    """Load and start pattern A, a loop of message EvtA and block BlockA.
    Stop the pattern and remove EvtA.
    The result is a schedule with only block BlockA.
    Then add node EvtA of type switch.
    snoop catches 12 timing messages.
    """
    snoopFile = 'snoop_remove4.csv'
    self.scheduleFile0 = 'remove4-0.dot'
    self.scheduleFile1 = 'remove4-1.dot'
    self.scheduleFile2 = 'remove4-2.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.downloadFile2 = self.scheduleFile2.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove4, duration=3)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1), [0], 0, 0)
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2), [0], 0, 0)
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove4(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    Add part of the schedule with a switch node instead of tmsg.
    """
    self.delay(0.3)
    self.startPattern(self.scheduleFile0, 'X')
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'startpattern', 'A'])
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    lines = self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], [0], 0, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.addSchedule(self.scheduleFile2)
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'startpattern', 'Y'])

  def testSchedRemovePatternA(self):
    scheduleFile1 = 'patternA-repcount.dot'
    scheduleFile2 = 'patternA-remove.dot'
    downloadFile0 = 'patternA-download.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + scheduleFile1), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'Pattern_A'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile2), [250], 2, 40)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'Pattern_A'), [0], 0, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile2), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + downloadFile0, downloadFile0), [0], 0, 0)
    self.deleteFile(downloadFile0)

  def testSchedRemoveTwoPattern(self):
    scheduleFile0 = 'patternApatternB.dot'
    scheduleFile1 = 'patternAremoveB.dot'
    downloadFile0 = scheduleFile0.replace('.dot', '-download.dot')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + scheduleFile0), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile1), [0], 0, 0)
    # ~ self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'Pattern_A'), [0], 0, 0)
    # ~ self.delay(1.0)
    # ~ self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile2), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + downloadFile0, downloadFile0), [0], 0, 0)
    self.deleteFile(downloadFile0)
