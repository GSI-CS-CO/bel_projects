import dm_testbench

"""Start all pattern in the schedules and download status. Compare original schedule with downloaded.
Steps for a schedule:
Add schedule to datamaster
Start all patterns of this schedule
Download status of datamaster
Compare original schedule with downloaded schedule
"""
class AddDownloadCompare(dm_testbench.DmTestbench):
  def addDownloadCompareSchedule(self, scheduleFile):
    status_file = 'status.dot'
    self.startAllPattern(scheduleFile)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', status_file))
    self.startAndCheckSubprocess(('scheduleCompare', self.schedules_folder + scheduleFile, status_file))
    self.deleteFile(status_file)

  def test_aScheduleTmsg1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-block-defdst.dot')

  def test_aScheduleBlock1(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-blockalign-altdst.dot')

  def test_aScheduleSwitch1(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-tmsg-defdst.dot')

  def test_aScheduleFlow1(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-tmsg-defdst.dot')

  def test_aScheduleFlush1(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-tmsg-defdst.dot')
