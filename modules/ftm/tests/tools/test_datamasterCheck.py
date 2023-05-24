import difflib
import pathlib
import unittest
import subprocess

"""Class collects tests for the datamasterCheck.sh script.
Test each of the known hosts.
"""
class DatamasterCheck(unittest.TestCase):

  def test_datamasterACOPC042(self):
    """Check ACOPC042
    """
    fileName = 'checkResultsACOPC042.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run('./datamasterCheck.sh', shell=True, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[38,38,42,59,77,77,109,120,155])
    self.deleteFile(fileName)

  def test_datamasterRemoteFel0069(self):
    """Check fel0069.acc
    """
    fileName = 'checkResultsFel0069.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'remote', 'fel0069.acc'), shell=False, check=False, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[11,11,11,14,31,31,50,50])
    self.deleteFile(fileName)

  def compareExpectedResult(self, fileCurrent, fileExpected, delete=[]):
    """Compare a file with a test result with an expected result contained in <fileExpected>.
    Lines numberered in delete are removed from <fileCurrent>  and <fileExpected> before checking.
    Assert that a unified diff has no lines.
    """
    with open(fileCurrent, 'r') as f_current:
      current = f_current.readlines()
    for index in delete:
      if index < len(current):
        del current[index]
    with open(fileExpected, 'r') as f_expected:
      expected = f_expected.readlines()
    for index in delete:
      if index < len(expected):
        del expected[index]
    diffLines = list(difflib.unified_diff(current, expected, n=0))
    self.assertEqual(len(diffLines), 0, f'Diff: file {fileExpected}\n{diffLines}')

  def deleteFile(self, fileName):
    """Delete file <fileName>.
    """
    fileToRemove = pathlib.Path(fileName)
    if fileToRemove.exists():
      fileToRemove.unlink()
