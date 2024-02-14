import dm_testbench

"""Test a bunch of cases for dm-sched overwrite.
"""
class DmSchedOverwrite(dm_testbench.DmTestbench):

  def testOverwrite1(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and overwrite the block with a blockalign.
    """
    snoopFile = 'snoop_overwrite1.csv'
    self.scheduleFile0 = 'overwrite1-0.dot'
    self.scheduleFile1 = 'overwrite1-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionOverwrite1, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10', '0x0000000000000002': '>5'})
    self.deleteFile(snoopFile)

  def actionOverwrite1(self):
    """During snoop start pattern A. This produces messages with 10Hz.
    Download the schedule for later compare.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(1.0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'overwrite', self.schedulesFolder + self.scheduleFile1], [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])

  def testOverwrite2(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and remove parts of the pattern which is nearly the same, but
    the block has no type. The result is a schedule with only the block.
    """
    snoopFile = 'snoop_overwrite2.csv'
    self.scheduleFile0 = 'overwrite2-0.dot'
    self.scheduleFile1 = 'overwrite2-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionOverwrite2, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10'})
    self.deleteFile(snoopFile)

  def actionOverwrite2(self):
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
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'overwrite', self.schedulesFolder + self.scheduleFile1], [250], 2, 2)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
