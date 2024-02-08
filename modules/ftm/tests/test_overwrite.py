import dm_testbench

"""Test a bunch of cases for dm-sched overwrite.
"""
class DmSchedOverwrite(dm_testbench.DmTestbench):

  def test_overwrite1(self):
    snoopFile = 'snoop_overwrite1.csv'
