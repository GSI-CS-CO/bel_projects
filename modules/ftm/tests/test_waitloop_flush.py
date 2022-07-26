import dm_testbench
import pytest

"""Class tests a wait loop terminated by the flush command.
"""
class UnitTestWaitloopFlush(dm_testbench.DmTestbench):

  def test_waitloop_flush(self):
    self.addSchedule('waitloop_flush.dot')
    # flush low prio queue with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'A'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0, 'BLOCK_LOOP', 1, checkFlush=False)
    self.delay(0.9)
    self.check_queue_flushed(0x1, 'BLOCK_LOOP', 1)
