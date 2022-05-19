import dm_testbench

"""
Start a pattern and check with saft-ctl snoop if FID 7 occurs.
When FID 7 occurs, the test failed.
"""
class Fid7(dm_testbench.DmTestbench):
  def test_fid7(self):
    fileName = 'snoop_protocol.csv'
    self.startPattern('fid.dot')
    self.snoopToCsv(fileName, 2)
    # analyse column 4 which contains the fid.
    # check that fid 0x1 occurs and that fid 0x7 does not occur.
    self.analyseFrequencyFromCsv(fileName, column=4, printTable=True, checkValues={'0x1': '>38', '0x7': '=0'})
    self.deleteFile(fileName)
