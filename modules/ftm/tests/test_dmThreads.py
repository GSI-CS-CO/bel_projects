import dm_testbench
import pytest

"""
Class starts dmThread tests and compares with expected result.
"""
class UnitTestDatamasterThreads(dm_testbench.DmTestbench):

  def test_dmThreads(self):
    self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'reset', 'all'), [0])
    self.startAndCheckSubprocess((self.binary_dm_sched, self.datamaster, 'clear', '-f'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '0'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '1'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '2'), [0])
    self.startAndCheckSubprocess(('eb-reset', self.datamaster, 'cpureset', '3'), [0])
    self.delay(1.0)
    self.addSchedule('../dmThreads/pps-all-threads.dot')
    threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3')]
    # ~ threadList = [('a', '0'), ('b', '1'), ('c', '2'), ('d', '3'), ('e', '4'), ('f', '5'), ('g', '6'), ('h', '7')]
    for x, y in threadList:
      self.startAndCheckSubprocess((self.binary_dm_cmd, self.datamaster, 'startpattern', 'PPS0' + x, '-t', y), [0])
    output = self.startAndGetSubprocessOutput((self.binary_dm_cmd, self.datamaster), [0], 13 + len(threadList))
