import common_scheduleCompare
import subprocess
import difflib
import pathlib
import os

"""Class tests replace chain operation of replaceChain.
"""
class TestReplaceChain(common_scheduleCompare.CommonScheduleCompare):
  @classmethod
  def setUpClass(self):
    """
    Set up for all test cases: store the environment variables in variables.
    """
    self.binary = os.environ.get('TEST_BINARY_SCHEDULECOMPARE', 'scheduleCompare').replace("scheduleCompare", "replaceChain")

  def callReplaceChain(self, arguments, expectedReturnCode=-1, linesCout=-1, linesCerr=-1):
    """
    Common method for test cases: run replaceChain.
    Start replaceChain with the arguments and check the output on stdout and stderr and the return code as well.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([*arguments], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    if expectedReturnCode > -1:
      self.assertEqual(process.returncode, expectedReturnCode,
        f'wrong return code {process.returncode}, Command line: {arguments}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {arguments}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {arguments}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')

  def compareExpectedResult(self, fileCurrent, fileExpected, exclude=''):
    """Compare a file with a test result with an expected result contained in <fileExpected>.
    Lines with the string <exclude> are removed from <fileCurrent> before checking against <fileExpected>.
    Assert that a unified diff has no lines.
    """
    with open(fileCurrent, 'r') as f_current:
      current = f_current.readlines()
    if len(exclude) > 0:
      current = [ x for x in current if exclude not in x ]
    with open(fileExpected, 'r') as f_expected:
      expected = f_expected.readlines()
    diffLines = list(difflib.unified_diff(current, expected, n=0))
    self.assertEqual(len(diffLines), 0, f'Diff: current file: {fileCurrent}, expected file: {fileExpected}\n{diffLines}')
    self.deleteFile(fileCurrent)

  def deleteFile(self, fileName):
    """Delete file <fileName>.
    """
    fileToRemove = pathlib.Path(fileName)
    if fileToRemove.exists():
      fileToRemove.unlink()

  def replaceChain(self, fileName, lines=2):
    """Replace all chains in the given schedule file.
    """
    self.callReplaceChain([self.binary, '-1', fileName], expectedReturnCode=0, linesCerr=0, linesCout=lines)
    self.compareExpectedResult('compact.dot', fileName.replace('.dot', '-chain-1.dot'))

  def replaceChain2(self, fileName, lines=2):
    """Replace all chains in the given schedule file. Use second version of replaceChain algorithm.
    """
    outputFileName = 'replace-chain.dot'
    self.callReplaceChain([self.binary, '-o', outputFileName, fileName], expectedReturnCode=0, linesCerr=0, linesCout=lines)
    self.compareExpectedResult(outputFileName, fileName.replace('.dot', '-chain-2.dot'))

  def test_replaceChainChain1(self):
    """Compact a one vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/chain1.dot', 6)

  def test_replaceChainChain3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/chain3.dot', 25)

  def test_replaceChainCycle3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/cycle3.dot', 36)

  def test_replaceChainCycle2x3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/cycle2x3.dot', 72)

  def test_compactChain1(self):
    """Compact a one vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/chain1.dot')

  def test_compactChain3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/chain3.dot')

  def test_compactCycle3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/cycle3.dot')

  def test_compactCycle2x3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/cycle2x3.dot', 3)

  def test_compactParallel1(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/parallel1.dot')

  def test_compactParallel2(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/parallel2.dot', 3)

  def test_compactStar4(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/star4.dot', 4)

  def test_compactTsl020Sis100(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChain('replaceChain/tsl020-sis100.dot', 4)

  def test_compactGitLog(self):
    """Compact a graph from git log.
    """
    self.replaceChain('replaceChain/git-dot.dot', 616)

  def test_compactGitLog50(self):
    """Compact a graph from git log.
    """
    self.replaceChain('replaceChain/git-dot-50.dot', 7)
