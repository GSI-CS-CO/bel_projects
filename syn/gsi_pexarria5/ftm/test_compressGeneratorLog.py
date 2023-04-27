import pathlib
import subprocess
import unittest

class TestCompressScript(unittest.TestCase):
  
  testFolder = 'testCompress/'
  fileName1 = 'file1'
  fileName2 = 'file2'
  folderName = 'folder1'
  folderDoesNotExist = 'nonExistentFolder'
  
  def setUp(self):
    folderToCreate = pathlib.Path(self.testFolder)
    folderToCreate.mkdir(parents=False, exist_ok=False)
    file1ToCreate = pathlib.Path(self.testFolder + self.fileName1)
    self.startAndGetSubprocessOutput(['touch', '--date=-72 hours', self.testFolder + self.fileName1], [0], 0, 0)
    file2ToCreate = pathlib.Path(self.testFolder + self.fileName2)
    file2ToCreate.touch(exist_ok=False)
    # create this as a file for the test (in compressGeneratorLog.sh) that the folder exists
    file3ToCreate = pathlib.Path(self.folderName)
    file3ToCreate.touch(exist_ok=False)
    
  def tearDown(self):
    self.deleteFile(self.testFolder + self.fileName1)
    self.deleteFile(self.testFolder + self.fileName1 + '.gz')
    self.deleteFile(self.testFolder + self.fileName2)
    self.deleteFile(self.folderName)
    folderToRemove = pathlib.Path(self.testFolder)
    folderToRemove.rmdir()

  def startAndGetSubprocessOutput(self, argumentsList, expectedReturnCode=[-1], linesCout=-1, linesCerr=-1):
    """Common method to start a subprocess and check the return code.
    The <argumentsList> contains the binary to execute and all arguments in one list.
    Start the binary for the test step with the arguments and check the number of lines for stdout and stderr.
    Check that the return code is in a list of allowed return codes.
    Return both stdout, stderr as list of lines.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([*argumentsList], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    self.assertTrue(process.returncode in expectedReturnCode, f'wrong return code {process.returncode}, expected: {expectedReturnCode}, '
          + f'Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCerr > -1:
      lines = stderr.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCerr, f'wrong stderr, expected {linesCerr} lines, Command line: {argumentsList}\nstderr: {lines}\nstdout: {stdout.decode("utf-8").splitlines()}')
    if linesCout > -1:
      lines = stdout.decode('utf-8').splitlines()
      self.assertEqual(len(lines), linesCout, f'wrong stdout, expected {linesCout} lines, Command line: {argumentsList}\nstderr: {stderr.decode("utf-8").splitlines()}\nstdout: {lines}')
    return [stdout.decode("utf-8").splitlines(), stderr.decode("utf-8").splitlines()]

  def test_usage(self):
    output = self.startAndGetSubprocessOutput(['./compressGeneratorLog.sh'], [1], 0, 2)

  def test_not_a_folder(self):
    self.startAndGetSubprocessOutput(['./compressGeneratorLog.sh', self.folderName], [1], 0, 1)

  def test_wrong_folder(self):
    self.startAndGetSubprocessOutput(['./compressGeneratorLog.sh', self.folderDoesNotExist], [1], 0, 1)

  def test_production(self):
    self.startAndGetSubprocessOutput(['./compressGeneratorLog.sh', self.testFolder], [0], 4, 0)
    fileCompressed = pathlib.Path(self.testFolder + self.fileName1 + '.gz')
    self.assertTrue(fileCompressed.exists())
    fileNotCompressed = pathlib.Path(self.testFolder + self.fileName2)
    self.assertTrue(fileNotCompressed.exists())

  def test_testmode_compress(self):
    self.startAndGetSubprocessOutput(['./compressGeneratorLog.sh', self.testFolder, 'testmode'], [0], 8, 1)
    fileCompressed = pathlib.Path(self.testFolder + self.fileName1 + '.gz')
    self.assertTrue(fileCompressed.exists())
    fileNotCompressed = pathlib.Path(self.testFolder + self.fileName2)
    self.assertTrue(fileNotCompressed.exists())

  def test_testmode_uncompress(self):
    self.startAndGetSubprocessOutput(['gzip', self.testFolder + self.fileName1], [0], 0, 0)
    self.startAndGetSubprocessOutput(['./compressGeneratorLog.sh', self.testFolder, 'testmode'], [0], 3, 1)
    fileUnCompressed = pathlib.Path(self.testFolder + self.fileName1)
    self.assertTrue(fileUnCompressed.exists())
    fileNotCompressed = pathlib.Path(self.testFolder + self.fileName2)
    self.assertTrue(fileNotCompressed.exists())

# utilitie methods
  def deleteFile(self, fileName):
    """Delete file <fileName>.
    """
    fileToRemove = pathlib.Path(fileName)
    if fileToRemove.exists():
      fileToRemove.unlink()
