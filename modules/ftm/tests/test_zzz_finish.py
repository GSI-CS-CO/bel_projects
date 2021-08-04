import dm_testbench

"""Start a pps pattern.

Required: set up of DmTestbench class.
"""
class DmLastTest(dm_testbench.DmTestbench):

  def test_last_test(self):
    self.startPattern(self.datamaster, 'pps.dot')
