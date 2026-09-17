import dm_testbench
from pathlib import Path
import shutil

"""
Class starts singleEdgeTest and compares with expected result.
"""
class UnitTestSingleEdgeTest(dm_testbench.DmTestbench):

  def test_singleEdgeTest(self):
    # preparation: make binary
    output = self.startAndGetSubprocessStdout(('make', '-C', 'singleEdgeTest/'), [0])
    # create output folder
    folderName = 'dot/'
    Path(folderName).mkdir(exist_ok=True)
    # run singleEdgeTest
    output = self.startAndGetSubprocessStdout(['./singleEdgeTest/singleEdgeTest'], [0])
    # check stdout
    self.compareExpectedOutput(output, 'singleEdgeTest/expected-result.txt')
    # delete output folder
    shutil.rmtree(folderName)
