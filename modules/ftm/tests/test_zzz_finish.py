import dm_testbench

"""Start a pps pattern.

Required: set up of DmTestbench class.
"""
class DmLastTest(dm_testbench.DmTestbench):

  def test_last_test(self):
    fileName = 'snoop_pps_last.csv'
    self.startPattern('pps-zzz.dot')
    self.snoopToCsv(fileName, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x00d7 and 0x00cd occur.
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues={'0x00d7': '>0', '0x00cd': '>0'})
    # analyse column 8 which contains the parameter.
    # check that evtno 0x234 and 0x123 occur.
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues={'0x0000000000000234': '>0', '0x0000000000000123': '>0'})
    self.deleteFile(fileName)
