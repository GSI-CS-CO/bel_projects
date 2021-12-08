
import dm_testbench

"""
Start a pps pattern.

Required: set up of DmTestbench class.
"""
class DmPps(dm_testbench.DmTestbench):

  def test_pps(self):
    self.startPattern(self.datamaster, 'pps.dot')
