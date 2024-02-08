import dm_testbench

"""Test a bunch of cases for dm-sched remove.
"""
class DmSchedRemove(dm_testbench.DmTestbench):

  def testRemove1(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove the pattern which is nearly the same, but
    the block has no type. The result is a schedule with only the block.
    """
    snoopFile = 'snoop_remove1.csv'
    self.scheduleFile0 = 'remove1-0.dot'
    self.scheduleFile1 = 'remove1-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove1, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove1(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testRemove2(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove parts of the pattern which is nearly the same, but
    the block has no type. The result is a schedule with only the block.
    """
    snoopFile = 'snoop_remove2.csv'
    self.scheduleFile0 = 'remove2-0.dot'
    self.scheduleFile1 = 'remove2-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove2, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove2(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], [250], 2, 2)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testRemove3(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove the pattern which is nearly the same, but
    the block has no type. The result is a schedule with only the block.
    """
    snoopFile = 'snoop_remove3.csv'
    self.scheduleFile0 = 'remove3-0.dot'
    self.scheduleFile1 = 'remove3-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove3, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove3(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    lines = self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], [250], 2, 3)
    self.assertEqual(lines[1][1], "../bin/dm-sched: Failed to execute <remove>. Cause: Validation of Neighbourhood: Node 'EvtA' of type 'tmsg' cannot be childless")
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testRemove4(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove the pattern which is nearly the same, but
    the block has no type. The result is a schedule with only the block.
    Thena add a node of type switch.
    """
    snoopFile = 'snoop_remove4.csv'
    self.scheduleFile0 = 'remove4-0.dot'
    self.scheduleFile1 = 'remove4-1.dot'
    self.scheduleFile2 = 'remove4-2.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.downloadFile2 = self.scheduleFile2.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionRemove4, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2))
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionRemove4(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    Stop the pattern and remove part of the schedule.
    Add part of the schedule with a switch node instead of tmsg.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    lines = self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + self.scheduleFile1], [0], 0, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.addSchedule(self.scheduleFile2)
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])
