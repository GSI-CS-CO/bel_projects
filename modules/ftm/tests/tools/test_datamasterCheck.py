import difflib
import pathlib
import unittest
import subprocess

"""Class collects tests for the datamasterCheck.sh script.
Test each of the known hosts.
The script datamasterCheck.sh runs and the output is compared to an
expected output. Since some of the lines are variable, these lines are deleted
prior comparison. These lines contain timestamps, message counts or similar.
The list delete=[...] may contain duplicate line numbers since all lines
after the deleted line are shifted one position lower.
"""
class DatamasterCheck(unittest.TestCase):

  def test_datamasterACOPC042(self):
    """Check ACOPC042
    Variable lines of output, deleted before comparing with expected output.
38: WR_time - host_time [ms]:   -14.462
38: Current TAI: 2024-03-27 13:10:53 GMT (95511 us), 1711545053095511 us
43: FPGA uptime [h]: 0000000006.70
80: 2024-03-27T14:10:53+01:00
112: ║ DataMaster: dev/wbm1                                                                      │ ECA-Time: 0x17c0a12862935270 ns │ 2024-03-27 13:10:53                                                                                                                       ║
123: ║  1  │  0  │   yes   │      3491 │                                                 PPS_ZZZ │                                                EVT_PPS2 │                                                 PPS_ZZZ │                                                EVT_PPS1 ║
    """
    fileName = 'checkResultsACOPC042.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run('./datamasterCheck.sh', shell=True, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[38,38,43,80,112,123,147,160,160])
    self.deleteFile(fileName)

  def test_datamasterRemoteFel0069(self):
    """Check fel0069.acc
    Variable lines of output, deleted before comparing with expected output.
11: WR_time - host_time [ms]: -58115.988
11: Current TAI: 2024-03-27 13:10:05 GMT (487285 us), 1711545005487285 us
15: FPGA uptime [h]: 0000000021.32
33: 2024-03-27 13:10:05 GMT (720223 us), 1711545005720223 us
53: 2024-03-27T13:11:04+0000
85: ║ DataMaster: dev/wbm0                                                                      │ ECA-Time: 0x17c0a11d7b036100 ns │ 2024-03-27 13:10:06   ║
96: ║  1  │  0  │   yes   │     19474 │                                                PPS_TEST │                                                EVT_PPS1 ║
    """
    fileName = 'checkResultsFel0069.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'remote', 'fel0069.acc'), shell=False, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[11,11,16,34,54,87])
    self.deleteFile(fileName)

  def test_datamasterFel0090(self):
    """Check fel0090.acc
    Variable lines of output, deleted before comparing with expected output.
25: 2024-03-27T13:11:01+0000
    """
    fileName = 'checkResultsFel0090.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'fel0090.acc'), shell=False, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[25,57,60,60,122])
    self.deleteFile(fileName)

  def test_datamasterFel0101(self):
    """Check fel0101.acc
    Variable lines of output, deleted before comparing with expected output.
25: 2024-03-27T13:11:02+0000
57: ║ DataMaster: dev/wbm0                                                                      │ ECA-Time: 0x17c0a1335015bd18 ns │ 2024-03-27 13:11:40   ║
60: ║  0  │  0  │   yes   │ 419678599 │                        SIS18_SLOW_HADES_20220612_201349 │                    SIS18_SLOW_HADES_20220612_201349_061 ║
    """
    fileName = 'checkResultsFel0101.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'fel0101.acc'), shell=False, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[25,57,60])
    self.deleteFile(fileName)

  def test_datamasterTsl014(self):
    """Check tsl014.acc
    Variable lines of output, deleted before comparing with expected output.
11: WR_time - host_time [ms]: 36999.660
11: Current TAI: 2024-03-27 13:11:45 GMT (428965 us), 1711545105428965 us
16: FPGA uptime [h]: 0000000814.64
34: 2024-03-27 13:11:47 GMT (25320 us), 1711545107025319 us
54: 2024-03-27T13:11:10+0000
86: ║ DataMaster: dev/wbm0                                                                      │ ECA-Time: 0x17c0a13516ed1d60 ns │ 2024-03-27 13:11:47   ║
    """
    fileName = 'checkResultsTsl014.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'tsl014.acc'), shell=False, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[11,11,16,34,54,86])
    self.deleteFile(fileName)

  def test_datamasterRemoteTsl018(self):
    """Check tsl018.acc
    Variable lines of output, deleted before comparing with expected output.
25: 2024-03-27T13:11:05+0000
57: ║ DataMaster: dev/wbm0                                                                         │ ECA-Time: 0x17c0a133ebafea20 ns │ 2024-03-27 13:11:42 ║
    """
    fileName = 'checkResultsTsl018.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'remote', 'tsl018.acc'), shell=False, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[25,57])
    self.deleteFile(fileName)

  def t1est_datamasterRemoteTsl020(self):
    """Check tsl020.acc
    Variable lines of output, deleted before comparing with expected output.
25: 2024-03-27T13:11:06+0000
57: ║ DataMaster: dev/wbm0                                                                      │ ECA-Time: 0x17c0a1343eb2ba50 ns │ 2024-03-27 13:11:44   ║
60: ║  0  │  0  │   yes   │  95004699 │                              SIS18_FAST_TE_ESR_20220615 │                         SIS18_FAST_TE_ESR_20220615_EXIT ║
67: ║  1  │  0  │   yes   │   6958349 │                 SCRATCH_SC_CRYRING_FAST_20220615_154447 │ SCRATCH_SC_CRYRING_FAST_20220615_154447_DMBlk_SR_WaitLoop ║
74: ║  2  │  0  │   yes   │   5528855 │                            SA_20231207183703398_DEFAULT │                       SA_20231207183703398_DEFAULT_EXIT ║
80: ║  2  │  7  │    no   │         0 │                                               undefined │                                                    idle ║
    """
    fileName = 'checkResultsTsl020.txt'
    with open(fileName, 'wb') as file1:
      process = subprocess.run(('./datamasterCheck.sh', 'remote', 'tsl020.acc'), shell=False, check=True, stdout=file1)
    self.compareExpectedResult(fileName, 'expected/' + fileName, delete=[25,57,60,67,74])
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
        print(f'{index}: {current[index]}')
        del current[index]
    with open(fileExpected, 'r') as f_expected:
      expected = f_expected.readlines()
    for index in delete:
      if index < len(expected):
        print(f'{index}: {expected[index]}')
        del expected[index]
    for index, line in enumerate(current):
      current[index] = line.replace('make[1]:', 'make:')
    diffLines = list(difflib.unified_diff(current, expected, n=0))
    self.assertEqual(len(diffLines), 0, f'Diff: file {fileExpected}\n{diffLines}')

  def deleteFile(self, fileName):
    """Delete file <fileName>.
    """
    fileToRemove = pathlib.Path(fileName)
    if fileToRemove.exists():
      fileToRemove.unlink()
