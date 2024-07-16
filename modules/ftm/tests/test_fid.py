import dm_testbench

"""Tests with different fids.
"""
class Fid(dm_testbench.DmTestbench):

  def test_fid7(self):
    """Start a pattern and check with saft-ctl snoop if FID 7 occurs.
    When FID 7 occurs, the test failed.
    """
    snoopFile = 'snoop_fid7.csv'
    self.startPattern('fid.dot')
    self.snoopToCsv(snoopFile, duration=2)
    # analyse column 4 which contains the fid.
    # check that fid 0x1 occurs and that fid 0x7 does not occur.
    self.analyseFrequencyFromCsv(snoopFile, column=4, printTable=True, checkValues={'0x1': '>38', '0x7': '=0'})
    self.deleteFile(snoopFile)

  def test_fid0(self):
    """Test with fid 0.
    Other layout of the timing messages.
    Check for the expected exceptions.
    """
    snoopFile = 'snoop_fid0.csv'
    self.startPattern('fid0.dot')
    self.snoopToCsv(snoopFile, duration=1)
    # with fid = 0 we do not have column 20 with the parameter values.
    try:
      self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x1': '=0'})
    except AssertionError as instance:
      self.assertEqual(instance.args[0], '19 not greater than 20 : Column 20 does not exist. Maximal column 19. May be fid=1 should be used in schedule.', 'wrong exception')
    self.deleteFile(snoopFile)

  def test_fid1(self):
    """Test with fid 1.
    Standard layout of the timing messages.
    Check for the expected number of messages.
    """
    snoopFile = 'snoop_fid1.csv'
    self.startPattern('fid1.dot')
    self.snoopToCsv(snoopFile, duration=1)
    # analyse column 20 which contains the parameter.
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000001': '>90', '0x0000000000000002': '>90'})
    self.deleteFile(snoopFile)
