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
    stderrLines = stderr.decode('utf-8').splitlines()
    stdoutLines = stdout.decode('utf-8').splitlines()
    if expectedReturnCode > -1:
      self.assertEqual(process.returncode, expectedReturnCode,
        f'wrong return code {process.returncode}, Command line: {arguments}\nstderr: {stderrLines}\nstdout: {stdoutLines}')
    if linesCerr > -1:
      self.assertEqual(len(stderrLines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {arguments}\nstderr: {stderrLines}\nstdout: {stdoutLines}')
    if linesCout > -1:
      self.assertEqual(len(stdoutLines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {arguments}\nstderr: {stderrLines}\nstdout: {stdoutLines}')
    return stdoutLines, stderrLines

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

  def test_version(self):
    """Test the version message.
    """
    self.callReplaceChain([self.binary, '-V'], expectedReturnCode=19, linesCerr=1, linesCout=0)

  def test_commandlineBlocksSeparated(self):
    """Test the command line parsing.
    """
    out, err = self.callReplaceChain([self.binary, '-vv', '-b', '-V'], expectedReturnCode=19, linesCerr=1, linesCout=17)
    self.assertTrue('blocksSeparated: 1' in out[5], f'blockSeparated not parsed: {out} .')

  def test_commandlineOverwrite(self):
    """Test the command line parsing.
    """
    out, err = self.callReplaceChain([self.binary, '-vv', '-w', '-V'], expectedReturnCode=19, linesCerr=1, linesCout=17)
    self.assertTrue('overwrite: 1' in out[10], f'overwrite not parsed: {out}.')

  def test_commandlineOutputFile(self):
    """Test the command line parsing.
    """
    out, err = self.callReplaceChain([self.binary, '-vv', '-o', 'file1.dot', '-V'], expectedReturnCode=19, linesCerr=1, linesCout=18)
    self.assertTrue('outputFile: file1.dot' in out[10], f'outputFile not parsed: {out}.')

  def test_commandlineFirstVersion(self):
    """Test the command line parsing.
    """
    out, err = self.callReplaceChain([self.binary, '-vv', '-1', '-V'], expectedReturnCode=19, linesCerr=1, linesCout=17)
    self.assertTrue('firstVersion: 1' in out[4], f'firstVersion not parsed: {out}.')

  def test_commandlineChainCount(self):
    """Test the command line parsing.
    """
    out, err = self.callReplaceChain([self.binary, '-vv', '-c', '17', '-V'], expectedReturnCode=19, linesCerr=1, linesCout=18)
    self.assertTrue('chainCount: 17' in out[8], f'chainCount not parsed: {out}.')

  def test_usage_message(self):
    """Test the usage message.
    """
    self.callReplaceChain([self.binary, '-h'], expectedReturnCode=14, linesCerr=23, linesCout=0)

  def replaceChain(self, fileName, lines=1):
    """Replace all chains in the given schedule file.
    """
    outputFileName = 'compact-chain.dot'
    self.callReplaceChain([self.binary, '-1swo', outputFileName, fileName], expectedReturnCode=0, linesCerr=0, linesCout=lines)
    self.compareExpectedResult(outputFileName, fileName.replace('.dot', '-chain-1.dot'))

  def replaceChain2(self, fileName, lines=2):
    """Replace all chains in the given schedule file. Use second version of replaceChain algorithm.
    """
    outputFileName = 'replace-chain.dot'
    self.callReplaceChain([self.binary, '-swo', outputFileName, fileName], expectedReturnCode=0, linesCerr=0, linesCout=lines)
    self.compareExpectedResult(outputFileName, fileName.replace('.dot', '-chain-2.dot'))

  def replaceChainBlocksSeparated(self, fileName, lines=2):
    """Replace all chains in the given schedule file. Use second version of replaceChain algorithm
    and put blocks in separate chains.
    """
    outputFileName = 'replace-chain-blocks.dot'
    self.callReplaceChain([self.binary, '-bswo', outputFileName, fileName], expectedReturnCode=0, linesCerr=0, linesCout=lines)
    self.compareExpectedResult(outputFileName, fileName.replace('.dot', '-chain-3.dot'))

  def test_replaceChainLoop(self):
    """Replace all chains in the chains1234r.dot schedule file.
    Replace chains one by one and compare the result with expected after
    each step.
    """
    fileName0 = 'replaceChain/chains1234r.dot'
    fileName1 = 'chains1234r-c1.dot'
    fileName2 = 'chains1234r-c2.dot'
    fileName3 = 'chains1234r-c3.dot'
    fileName4 = 'chains1234r-c4.dot'
    self.callReplaceChain([self.binary, '-c', '1', '-swo', fileName1, fileName0], expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callReplaceChain([self.binary, '-c', '1', '-swo', fileName2, fileName1], expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callReplaceChain([self.binary, '-c', '1', '-swo', fileName3, fileName2], expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callReplaceChain([self.binary, '-c', '1', '-swo', fileName4, fileName3], expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.compareExpectedResult(fileName1, 'replaceChain/' + fileName1)
    self.compareExpectedResult(fileName2, 'replaceChain/' + fileName2)
    self.compareExpectedResult(fileName3, 'replaceChain/' + fileName3)
    self.compareExpectedResult(fileName4, 'replaceChain/' + fileName4)

  def test_replaceChainChain1(self):
    """Compact a one vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/chain1.dot', 0)

  def test_replaceChainChain2x1(self):
    """Compact two one vertex chains to two single vertices.
    """
    self.replaceChain2('replaceChain/chain2x1.dot', 0)

  def test_replaceChainChain3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/chain3.dot', 0)

  def test_replaceChainChains123(self):
    """Compact a graph with three chains (1 vertex, 2 vertices, 3 vertices)
    to three chains with one vertex.
    """
    self.replaceChain2('replaceChain/chains123.dot', 0)

  def test_replaceChainChains1234(self):
    """Compact a graph with four chains (1 vertex, 2 vertices, 3 vertices, 4 vertices)
    to four chains with one vertex. Test the labeling.
    """
    self.replaceChain2('replaceChain/chains1234.dot', 0)

  def test_replaceChainChains1234r(self):
    """Compact a graph with four chains (1 vertex, 2 vertices, 3 vertices, 4 vertices)
    to four chains with one vertex. Test the labeling.
    Chains with reversed numbering (first number at end of chain).
    """
    self.replaceChain2('replaceChain/chains1234r.dot', 0)

  def test_replaceChainChains123123(self):
    """Compact a graph with six chains (2x1 vertex, 2x2 vertices, 2x3 vertices)
    to six chains with one vertex.
    """
    self.replaceChain2('replaceChain/chains123123.dot', 0)

  def test_replaceChainChains123123123(self):
    """Compact a graph with nine chains (3x1 vertex, 3x2 vertices, 3x3 vertices)
    to nine chains with one vertex.
    """
    self.replaceChain2('replaceChain/chains123123123.dot', 0)

  def test_replaceChainChains123123123Blocks(self):
    """Compact a graph with nine chains (3x1 vertex, 3x2 vertices, 3x3 vertices)
    to nine chains with one vertex.
    """
    self.replaceChainBlocksSeparated('replaceChain/chains123123123.dot', 0)

  def test_replaceChainChain2x3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/chain2x3.dot', 0)

  def test_replaceChainChainX(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/chainX.dot', 0)

  def test_replaceChainChainY(self):
    """Compact a five vertex chain to one vertex.
    Tests the labelling for a chain with more than three vertices.
    """
    self.replaceChain2('replaceChain/chainY.dot', 0)

  def test_replaceChainChain5(self):
    """Compact a five vertex chain to one vertex.
    Tests option -b on all vertices of the same type.
    """
    self.replaceChainBlocksSeparated('replaceChain/chain5.dot', 0)

  def test_replaceChainChain5Block1(self):
    """Compact a five vertex chain to two vertices. Top vertex is a block.
    Tests separating vertices with different types.
    """
    self.replaceChain2('replaceChain/chain5Block1.dot', 0)

  def test_replaceChainChain5Block1BlocksSeparated(self):
    """Compact a five vertex chain to two vertices. Top vertex is a block.
    Tests separating vertices with different types.
    """
    self.replaceChainBlocksSeparated('replaceChain/chain5Block1.dot', 0)

  def test_replaceChainChain5Block3(self):
    """Compact a five vertex chain to three vertices. Third vertex is a block.
    Tests separating vertices with different types.
    """
    self.replaceChain2('replaceChain/chain5Block3.dot', 0)

  def test_replaceChainChain5Block3BlocksSeparated(self):
    """Compact a five vertex chain to three vertices. Third vertex is a block.
    Tests separating vertices with different types.
    """
    self.replaceChainBlocksSeparated('replaceChain/chain5Block3.dot', 0)

  def test_replaceChainChain5Block5(self):
    """Compact a five vertex chain to two vertices. Bottom vertex is a block.
    Tests separating vertices with different types.
    """
    self.replaceChain2('replaceChain/chain5Block5.dot', 0)

  def test_replaceChainChain5Block5BlocksSeparated(self):
    """Compact a five vertex chain to two vertices. Bottom vertex is a block.
    Tests separating vertices with different types.
    """
    self.replaceChainBlocksSeparated('replaceChain/chain5Block5.dot', 0)

  def test_replaceChainCycle3(self):
    """Compact a three vertex cycle into a two vertex cycle.
    """
    self.replaceChain2('replaceChain/cycle3.dot', 0)

  def test_replaceChainCycle10(self):
    """Compact a ten vertex cycle into a two vertex cycle.
    """
    self.replaceChain2('replaceChain/cycle10.dot', 0)

  def test_replaceChainCycle10BlockSeparated(self):
    """Compact a ten vertex cycle into a two vertex cycle.
    """
    self.replaceChainBlocksSeparated('replaceChain/cycle10.dot', 0)

  def test_replaceChainCycle10ExtraBlockSeparated(self):
    """Compact a ten vertex cycle into a two vertex cycle.
    """
    self.replaceChainBlocksSeparated('replaceChain/cycle10ExtraBlock.dot', 0)

  def test_replaceChainCycle10ExtraBlock2Separated(self):
    """Compact a ten vertex cycle into a two vertex cycle.
    """
    self.replaceChainBlocksSeparated('replaceChain/cycle10ExtraBlock2.dot', 0)

  def test_replaceChainCycle2x3(self):
    """Compact two three vertex cycles into two two vertex cycles.
    """
    self.replaceChain2('replaceChain/cycle2x3.dot', 0)

  def test_replaceChainEight(self):
    """Compact two four vertex cycles with a common vertex into two two vertex cycles.
    """
    self.replaceChain2('replaceChain/eight.dot', 0)

  def test_replaceChainParallel1(self):
    """Compact a three vertex chain and a parallel edge into the same (nothing to do).
    """
    self.replaceChain2('replaceChain/parallel1.dot', 0)

  def test_replaceChainParallel2(self):
    """Compact two parallel three vertex chains. Nothing to do.
    """
    self.replaceChain2('replaceChain/parallel2.dot', 0)

  def test_replaceChainParallel1x3(self):
    """Compact a five vertex chain and a parallel edge.
    """
    self.replaceChain2('replaceChain/parallel1x3.dot', 0)

  def test_replaceChainParallel1x3r(self):
    """Compact a five vertex chain and a parallel edge (reverse numbering of the vertices).
    """
    self.replaceChain2('replaceChain/parallel1x3r.dot', 0)

  def test_replaceChainParallel2x2(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain2('replaceChain/parallel2x2.dot', 0)

  def test_replaceChainParallel1x4(self):
    """Compact a six vertex chain and a parallel edge.
    """
    self.replaceChain2('replaceChain/parallel1x4.dot', 0)

  def test_replaceChainParallel1x4r(self):
    """Compact a six vertex chain and a parallel edge (reverse numbering of the vertices).
    """
    self.replaceChain2('replaceChain/parallel1x4r.dot', 0)

  def test_replaceChainParallel1x4rBlocks(self):
    """Compact a six vertex chain and a parallel edge (reverse numbering of the vertices).
    """
    self.replaceChainBlocksSeparated('replaceChain/parallel1x4r.dot', 0)

  def test_replaceChainStar4(self):
    """Compact a four element star. Nothing to do.
    """
    self.replaceChain2('replaceChain/star4.dot', 0)

  def test_replaceChainBlockChain22(self):
    """Compact a four element chain with two block and two events.
    """
    self.replaceChain2('replaceChain/blockChain22.dot', 0)

  def test_replaceChainBlockChain22BlocksSeparated(self):
    """Compact a four element chain with two block and two events.
    Blocks separated.
    """
    self.replaceChainBlocksSeparated('replaceChain/blockChain22.dot', 0)

  def test_replaceChainEsrStacking(self):
    """Compact a schedule for ESR stacking (part of larger schedule).
    """
    self.replaceChain2('replaceChain/EsrStacking.dot', 0)

  def test_replaceChainEsrStackingBlocksSeparated(self):
    """Compact a schedule for ESR stacking (part of larger schedule).
    Blocks separated.
    """
    self.replaceChainBlocksSeparated('replaceChain/EsrStacking.dot', 0)

  def test_replaceChainEsrSnoopy(self):
    """Compact a schedule for ESR stacking (the larger schedule).
    """
    self.replaceChain2('replaceChain/snoopy.dot', 0)

  def test_replaceChainEsrSnoopyBlocksSeparated(self):
    """Compact a schedule for ESR stacking (the larger schedule).
    Blocks separated.
    """
    self.replaceChainBlocksSeparated('replaceChain/snoopy.dot', 0)

  def test_replaceChainTsl020Sis100(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChain2('replaceChain/tsl020-sis100.dot', 0)

  def test_replaceChainTsl020Sis100BlocksSeparated(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChainBlocksSeparated('replaceChain/tsl020-sis100.dot', 0)

  def test_replaceChainTsl020(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChain2('replaceChain/tsl020.dot', 0)

  def test_replaceChainTsl017(self):
    """Compact a schedule from tsl017.
    """
    self.replaceChain2('replaceChain/tsl017.dot', 0)

  def test_replaceChainTsl020BlocksSeparated(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChainBlocksSeparated('replaceChain/tsl020.dot', 0)

  def test_replaceChainTsl017BlocksSeparated(self):
    """Compact a schedule from tsl017.
    """
    self.replaceChainBlocksSeparated('replaceChain/tsl017.dot', 0)

  def test_replaceChainGitLog(self):
    """Compact a graph from git log.
    """
    self.replaceChain2('replaceChain/git-dot.dot', 0)

  def test_replaceChainGitLog50(self):
    """Compact a graph from git log.
    """
    self.replaceChain2('replaceChain/git-dot-50.dot', 0)

  def test_compactChain1(self):
    """Compact a one vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/chain1.dot')

  def test_compactChain2x1(self):
    """Compact two one vertex chains to two single vertices.
    Test that the algorithm finds more than one chain.
    """
    self.replaceChain('replaceChain/chain2x1.dot', 2)

  def test_compactChain3(self):
    """Compact a three vertex chain to one vertex.
    """
    self.replaceChain('replaceChain/chain3.dot')

  def test_compactChains123(self):
    """Compact a graph with three chains (1 vertex, 2 vertices, 3 vertices)
    to three chains with one vertex.
    """
    self.replaceChain('replaceChain/chains123.dot', 3)

  def test_compactChains123123(self):
    """Compact a graph with six chains (2x1 vertex, 2x2 vertices, 2x3 vertices)
    to six chains with one vertex.
    """
    self.replaceChain('replaceChain/chains123123.dot', 6)

  def test_compactChains123123123(self):
    """Compact a graph with nine chains (3x1 vertex, 3x2 vertices, 3x3 vertices)
    to nine chains with one vertex.
    """
    self.replaceChain('replaceChain/chains123123123.dot', 9)

  def test_compactChain2x3(self):
    """Compact two three vertex chain to two single vertices.
    """
    self.replaceChain('replaceChain/chain2x3.dot', 2)

  def test_compactCycle3(self):
    """Compact a three vertex cycle into a two vertex cycle.
    """
    self.replaceChain('replaceChain/cycle3.dot')

  def test_compactCycle2x3(self):
    """Compact two three vertex cycles into two two vertex cycles.
    """
    self.replaceChain('replaceChain/cycle2x3.dot', 2)

  def test_compactParallel1(self):
    """Compact a three vertex chain and a parallel edge into the same (nothing to do).
    """
    self.replaceChain('replaceChain/parallel1.dot')

  def test_compactParallel2(self):
    """Compact two parallel three vertex chains. Nothing to do.
    """
    self.replaceChain('replaceChain/parallel2.dot', 2)

  def test_compactParallel2x2(self):
    """Compact two four vertex chains to two three vertex chains.
    """
    self.replaceChain('replaceChain/parallel2x2.dot', 2)

  def test_compactParallel1x3(self):
    """Compact a five vertex chain and a parallel edge.
    """
    self.replaceChain('replaceChain/parallel1x3.dot', 1)

  def test_compactParallel1x3r(self):
    """Compact a five vertex chain and a parallel edge (reverse numbering of the vertices).
    """
    self.replaceChain('replaceChain/parallel1x3r.dot', 1)

  def test_compactParallel1x4(self):
    """Compact a six vertex chain and a parallel edge.
    """
    self.replaceChain('replaceChain/parallel1x4.dot', 1)

  def test_compactParallel1x4r(self):
    """Compact a six vertex chain and a parallel edge (reverse numbering of the vertices).
    """
    self.replaceChain('replaceChain/parallel1x4r.dot', 1)

  def test_compactStar4(self):
    """Compact a four element star. Nothing to do.
    """
    self.replaceChain('replaceChain/star4.dot', 3)

  def test_compactTsl020Sis100(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChain('replaceChain/tsl020-sis100.dot', 3)

  def test_compactTsl020(self):
    """Compact a schedule from tsl020.
    """
    self.replaceChain('replaceChain/tsl020.dot', 61)

  def test_compactGitLog(self):
    """Compact a graph from git log.
    """
    self.replaceChain('replaceChain/git-dot.dot', 615)

  def test_compactGitLog50(self):
    """Compact a graph from git log.
    """
    self.replaceChain('replaceChain/git-dot-50.dot', 6)
