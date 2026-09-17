import common_scheduleCompare

"""
Class collects unit tests for scheduleCompare.
Tests run scheduleCompare with two dot files with option -u and check the result.
"""
class TestScheduleCompareOptionU(common_scheduleCompare.CommonScheduleCompare):

  def test_OptionU(self):
    self.callScheduleCompare('schedules/pps-subgraph.dot', 'schedules/pps-undefined.dot', '-u', expectedReturnCode=0, linesCerr=0, linesCout=3)

  def test_withoutOptionU(self):
    self.callScheduleCompare('schedules/pps-subgraph.dot', 'schedules/pps-undefined.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)
