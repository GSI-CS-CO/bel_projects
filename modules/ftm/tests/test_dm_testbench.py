import pytest
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
    with pytest.raises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '<1'.
  """
  def test_analyseFrequencyFromCsv1(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x1111': '<1'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '=0'.
  """
  def test_analyseFrequencyFromCsv2(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x1111': '=0'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '=n'.
  """
  def test_analyseFrequencyFromCsv3(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00c3': '=1'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '>n'.
  """
  def test_analyseFrequencyFromCsv4(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '>990'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax 'n'.
  """
  def test_analyseFrequencyFromCsv5(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '999'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '<n'.
  """
  def test_analyseFrequencyFromCsv6(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x00a1': '<1000'}
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)

  """Test syntax '<n' with missing key.
  """
  def test_analyseFrequencyFromCsv7(self):
    fileName = 'other/snoop_test_analyseFrequencyFromCsv.csv'
    checkValues1 = {'0x1111': '<5'}
    with pytest.raises(AssertionError):
      self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues=checkValues1)
