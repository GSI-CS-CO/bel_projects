import dm_testbench

"""Test a bunch of cases for dm-sched overwrite.
"""
class DmSchedOverwrite(dm_testbench.DmTestbench):

  def testOverwrite1(self):
    """Load and start pattern A, a loop of a message and a block.
    Stop the pattern and overwrite the block with a blockalign.
    Analyse the timing messages with the parameter field.
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
    """Load a schedule with a switch. Executing the switch interchanges
    the defdst to EvtA with the altdst to EvtB. Then the schedule loops
    over EvtB.
    Overwrite the schedule with the original schedule changes the edges back
    to defdst to EvtA and altdst to EvtB. Then the switch is executed again.
    Analyse the timing messages with the parameter field.
    """
    snoopFile = 'snoop_overwrite2.csv'
    self.scheduleFile0 = 'overwrite2-0.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = 'overwrite2-1-download.dot'
    self.downloadFile2 = 'overwrite2-2-download.dot'
    self.snoopToCsvWithAction(snoopFile, self.actionOverwrite2, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2))
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '2', '0x0000000000000002': '>10'})
    self.deleteFile(snoopFile)

  def actionOverwrite2(self):
    """During snoop:
    Add a schedule, save the status for later compare.
    Start pattern A and save the status for later compare.
    Stop the pattern.
    Overwrite the schedule with the original one and save the status for later compare.
    """
    self.addSchedule(self.scheduleFile0)
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(0.5)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'overwrite', self.schedulesFolder + self.scheduleFile0], [0], 0, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)

  def testOverwrite3(self):
    """Load a schedule with a switch. Executing the switch interchanges
    the defdst to EvtA with the altdst to EvtB. Then the schedule loops
    over EvtB.
    Overwrite the schedule with a smiliar schedule which replaces the switch
    with a flow and the switchdst edge with a flowdst edge.
    Analyse the timing messages with the parameter field.
    """
    snoopFile = 'snoop_overwrite3.csv'
    self.scheduleFile0 = 'overwrite3-0.dot'
    self.scheduleFile1 = 'overwrite3-1.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.downloadFile1 = self.scheduleFile1.replace('.dot', '-download.dot')
    self.downloadFile2 = 'overwrite3-2-download.dot'
    self.snoopToCsvWithAction(snoopFile, self.actionOverwrite3, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0))
    self.deleteFile(self.downloadFile0)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile1, self.downloadFile1))
    self.deleteFile(self.downloadFile1)
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + self.downloadFile2, self.downloadFile2))
    self.deleteFile(self.downloadFile2)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>10', '0x0000000000000002': '>5'})
    self.deleteFile(snoopFile)

  def actionOverwrite3(self):
    """During snoop:
    Add a schedule, save the status for later compare.
    Start pattern A and save the status for later compare.
    Stop the pattern.
    Overwrite the schedule with the flow schedule and save the status for later compare.
    """
    self.addSchedule(self.scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
    self.delay(0.5)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile1])
    self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'])
    self.delay(0.1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'overwrite', self.schedulesFolder + self.scheduleFile1], [0], 0, 0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile2])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0], 1, 0)
