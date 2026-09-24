import dm_testbench

"""Class collects unit tests for coupling.
"""
class UnitTestCoupling(dm_testbench.DmTestbench):

  def test_static_coupling(self):
    """Load first the schedule static-coupling-schedule0.dot and then
    the coupled schedule static-coupling-schedule1.dot.
    """
    self.addSchedule('static-coupling-schedule0.dot')
    self.addSchedule('static-coupling-schedule1.dot')
    download_file = 'download.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', download_file), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', self.schedulesFolder + 'static-coupling-comp.dot', download_file), [0], 0, 0)
    self.deleteFile(download_file)
