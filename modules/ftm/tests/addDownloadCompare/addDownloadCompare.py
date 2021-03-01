#! /usr/bin/env python3

import dm_testbench
import sys
import unittest

"""
Start all pattern in the schedules and download status. Compare original schedule with downloaded.
Steps for a schedule:
Add schedule to datamaster
Start all patterns of this schedule
Download status of datamaster
Compare original schedule with downloaded schedule

Required argument: device of data master.
"""
class AddDownloadCompare(dm_testbench.DmTestbench):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.datamaster = test_datamaster

  def test_aSchedule(self):
    schedule_file = 'testSingleEdge-block-blockalign-altdst.dot'
    schedule_file = 'testSingleEdge-tmsg-block-defdst.dot'
    self.startAllPattern(self.datamaster, schedule_file)
    self.startAndCheckSubprocess('dm-sched', self.datamaster, 'status', '-ostatus.dot', expectedReturnCode=0)
    self.startAndCheckSubprocess('scheduleCompare', schedule_file, 'status.dot')

if __name__ == '__main__':
  if len(sys.argv) > 1:
    test_datamaster = sys.argv.pop()
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)

