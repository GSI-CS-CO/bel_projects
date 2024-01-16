import dm_testbench

"""Test to overwrite a block with a block which has other priority queues.
"""
class OverwriteBlocks(dm_testbench.DmTestbench):

  def actionOverwriteSchedules(self):
    self.delay(0.1)
    # start the first pattern with no queues
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS_Q'), linesCout=1, linesCerr=0)
    self.delay(2)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'abort'), linesCout=1, linesCerr=0)
    # overwrite and start the second pattern with low prio queue
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'overwrite', self.schedules_folder + 'pps-qlo.dot'), linesCout=0, linesCerr=0)
    self.delay(0.1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS_Q'), linesCout=1, linesCerr=0)
    self.delay(2)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'PPS_Q'), linesCout=0, linesCerr=0)
    self.checkSchedule('pps-qlo-status.dot')
    # overwrite and start the second pattern with low and high prio queue
    self.delay(0.1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'overwrite', self.schedules_folder + 'pps-qhi.dot'), linesCout=0, linesCerr=0)
    self.delay(0.1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PPS_Q'), linesCout=1, linesCerr=0)
    self.delay(2.5)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'PPS_Q'), linesCout=0, linesCerr=0)
    self.checkSchedule('pps-qhi-status.dot')

  def checkSchedule(self, scheduleFile):
    statusFile = 'status.dot'
    options = '-so'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', options, statusFile))
    self.startAndCheckSubprocess(('scheduleCompare', self.schedules_folder + scheduleFile, statusFile))
    self.deleteFile(statusFile)

  def test_overwriteSchedules(self):
    fileName = 'snoop_overwriteSchedules.csv'
    self.addSchedule('pps-no-queue.dot')
    self.snoopToCsvWithAction(fileName, self.actionOverwriteSchedules, duration=8)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x00d7 and 0x00cd occur.
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues={'0x00d7': '>0', '0x00cd': '>0'})
    # analyse column 20 which contains the parameter.
    # check that parameters 1, 2, 11, 12, 21, and 22 occur.
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True,
        checkValues={'0x0000000000000001': '>20', '0x0000000000000002': '>20', '0x000000000000000b': '>20', '0x000000000000000c': '>20', '0x0000000000000015': '>20', '0x0000000000000016': '>20'})
    self.deleteFile(fileName)
