import dm_testbench

"""
Start all pattern in the schedules and download status. Compare original schedule with downloaded.
Steps for a schedule:
Add schedule to datamaster
Start all patterns of this schedule
Download status of datamaster
Compare original schedule with downloaded schedule
"""
class AddDownloadCompare(dm_testbench.DmTestbench):
  def test_aSchedule(self):
    schedule_file = 'testSingleEdge-block-blockalign-altdst.dot'
    schedule_file = 'testSingleEdge-tmsg-block-defdst.dot'
    status_file = 'status.dot'
    self.startAllPattern(self.datamaster, schedule_file)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', status_file))
    self.startAndCheckSubprocess(('../analysis/scheduleCompare/main/scheduleCompare', self.schedules_folder + schedule_file, status_file))
    self.deleteFile(status_file)

