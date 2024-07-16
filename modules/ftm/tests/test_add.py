import dm_testbench

"""
Tests with dm-sched <datamaster> add <schedule>.

Required: set up of DmTestbench class.
"""
class DmSchedAdd(dm_testbench.DmTestbench):

  def testSchedAdd(self):
    scheduleFile1 = 'patternA-v1.dot'
    scheduleFile2 = 'patternA-v2.dot'
    downloadFile0 = 'patternA-v1-v2.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + scheduleFile1), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + scheduleFile2), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + downloadFile0, downloadFile0), [0], 0, 0)
    self.deleteFile(downloadFile0)
