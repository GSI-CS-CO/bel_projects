import dm_testbench

"""Test dm_testbench.py.
    checkValues is a dictionary of key-value pairs to check. Key is a value in the column and value is the required frequency.
    The value can be '>n', '<n', '=n', 'n' (which is the same as '=n'), '=0'. The syntax '<n' fails if there are no occurrences.
    Checks for intevalls are not possible since checkValues is a dictionary and keys occur at most once.
    Example: column=8 and checkValues={'0x0001': 62} checks that EVTNO 0x0001 occurs in 62 lines of the file to analyse.
    Example: column=8 and checkValues={'0x0002': '>0'} checks that EVTNO 0x0002 occurs at least once in the file to analyse.
    Example: column=4 and checkValues={'0x7': '=0'} checks that FID 0x7 does NOT occur in the file to analyse.

"""
class DmTestTestbench(dm_testbench.DmTestbench):

  """Test missing key 0x00b2.
  """
  def test_analyseFrequencyFromCsv0(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '>990', '0x00a2': '1', '0x00a3': '1',
                '0x00b1': '>990', '0x00b2': '1', '0x00b3': '1',
                '0x00c1': '>990', '0x00c2': '1', '0x00c3': '1'}
    with self.assertRaises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '<1'.
  """
  def test_analyseFrequencyFromCsvLessOne(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x1111': '<1'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '=0'.
  """
  def test_analyseFrequencyFromCsvEqualZero(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x1111': '=0'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '=n'.
  """
  def test_analyseFrequencyFromCsvEqual(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00c3': '=1'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '>n'.
  """
  def test_analyseFrequencyFromCsvGreater(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '>990'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax 'n'.
  """
  def test_analyseFrequencyFromCsvExact(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '999'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '<n'.
  """
  def test_analyseFrequencyFromCsvLess(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '<1000'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '<n' with missing key.
  """
  def test_analyseFrequencyFromCsvMissingKey(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x1111': '<5'}
    with self.assertRaises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  def test_analyseFrequencyFromCsvAddDelayedFalseGreater(self):
    """Test handling of 'delayed' in snoop file. Default: count delayed
    messages separate. AssertionError is expected.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000001': '>50'}
    with self.assertRaises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1)

  def test_analyseFrequencyFromCsvAddDelayedTrueGreater(self):
    """Test handling of 'delayed' in snoop file. E.g. count delayed messages
    as normal messages. Setting addDelayed = True.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000001': '>50'}
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1, addDelayed=True)

  def test_analyseFrequencyFromCsvAddDelayedFalseLess(self):
    """Test handling of 'delayed' in snoop file. Default: count delayed
    messages separate. AssertionError is expected.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000001': '<51'}
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1)

  def test_analyseFrequencyFromCsvAddDelayedTrueLess(self):
    """Test handling of 'delayed' in snoop file. E.g. count delayed messages
    as normal messages. Setting addDelayed = True.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000001': '<51'}
    with self.assertRaises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1, addDelayed=True)

  def test_analyseFrequencyFromCsvAddDelayedFalseEqual(self):
    """Test handling of 'delayed' in snoop file. Default: count delayed
    messages separate. AssertionError is expected.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000001': '=50'}
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1)

  def test_analyseFrequencyFromCsvAddDelayedTrueEqual(self):
    """Test handling of 'delayed' in snoop file. E.g. count delayed messages
    as normal messages. Setting addDelayed = True.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000001': '=50'}
    with self.assertRaises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1, addDelayed=True)

  def test_analyseFrequencyFromCsvAddDelayedOkKey2(self):
    """Test handling of 'delayed' in snoop file. Default: count delayed
    messages separate. For key 0x0000000000000002 we have no delayed messages.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000002': '>50'}
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1)

  def test_analyseFrequencyFromCsvAddDelayedExceptionKey2(self):
    """Test handling of 'delayed' in snoop file. E.g. count delayed messages
    as normal messages. Setting addDelayed = True. For key
    0x0000000000000002 we have no delayed messages. So we get the
    expected AssertionError since we have not 61 or more messages.
    """
    fileName = 'other/snoop_simultaneousThreads8.csv'
    checkValues1 = {'0x0000000000000002': '>60'}
    with self.assertRaises(AssertionError, msg='AssertionError: 52 not greater than 60 : assertGreater for 0x0000000000000002: is:52 expected:>60'):
      self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues=checkValues1, addDelayed=True)

  def test_analyseFrequencyFromCsvAllDelayed(self):
    """Test handling of 'delayed' in snoop file with addDelayed=False.
    This snoop file has ony 0x0000000000000000!delayed, not
    0x0000000000000000 as key. So we get the expected AssertionError.
    """
    snoopFile = 'other/snoop_TwoCpusSwitch.csv'
    checkValues1 = {'0x0000000000000000': '>35', '0x0000000000000001': '>0', '0x0000000000000002': '>35'}
    with self.assertRaises(AssertionError):
      self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues=checkValues1, addDelayed=False)

  def test_analyseFrequencyFromCsvAllDelayedTrue(self):
    """Test handling of 'delayed' in snoop file with addDelayed=True.
    This snoop file has ony 0x0000000000000000!delayed, not
    0x0000000000000000 as key. Thus, listCounters[item] raises a KeyError
    for item = 0x0000000000000000. This test tests the coding for only
    delayed messages for one key.
    """
    snoopFile = 'other/snoop_TwoCpusSwitch.csv'
    checkValues1 = {'0x0000000000000000': '=45', '0x0000000000000001': '>10', '0x0000000000000002': '>89'}
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues=checkValues1, addDelayed=True)

  def test_timestamp(self):
    self.printTimestamp("Test Text")
