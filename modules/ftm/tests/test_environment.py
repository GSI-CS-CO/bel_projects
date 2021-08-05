import dm_testbench
import subprocess
import re

"""Class shows the current test environment.
This includes:
    test which binaries are used by the tests for dm-cmd and dm-sched.
    test which libcarpedm is used.
The tests fail, if not the binaries from local build are used.
"""
class UnitEnvironment(dm_testbench.DmTestbench):

  def test_environment_dm_cmd(self):
    """This test checks, which dm-cmd is used.
    The test passes, if '../bin/dm-cmd' is used (the binary from the build environment).
    The test fails, if no dm-cmd is found or some other binary is used.
    If dm-cmd is not found, the failure is a FileNotFoundError and the output contains:
    "FileNotFoundError: [Errno 2] No such file or directory: '../bin/dm-cmd': '../bin/dm-cmd'"
    If the installed dm-cmd is used, the failure is an AssertionError and the output contains:
    "['Usage: dm-cmd [OPTION] <etherbone-device> <command> [target node] [parameter] ']"
    The important part is between 'Usage:' and '[Option]'. This is the used binary.
    """
    process = subprocess.run([self.binary_dm_cmd, '-h'], stderr=subprocess.PIPE, check=True, universal_newlines=True)
    findall = re.findall(r'../bin/dm-cmd', process.stderr)
    self.assertTrue(len(findall) > 0, f'Called dm-cmd: {re.findall("Usage:.*", process.stderr)}')
    self.assertEqual(findall[0], '../bin/dm-cmd', f'Called dm-cmd: {re.findall("Usage:.*", process.stderr)}')

  def test_environment_dm_sched(self):
    """This test checks, which dm-sched is used.
    The test passes, if '../bin/dm-sched' is used (the binary from the build environment).
    The test fails, if no dm-sched is found or some other binary is used.
    If dm-sched is not found, the failure is a FileNotFoundError and the output contains:
    "FileNotFoundError: [Errno 2] No such file or directory: '../bin/dm-sched': '../bin/dm-sched'"
    If the installed dm-sched is used, the failure is an AssertionError and the output contains:
    "['Usage: dm-sched [OPTION] <etherbone-device> <command> [target node] [parameter] ']"
    The important part is between 'Usage:' and '[Option]'. This is the used binary.
    """
    process = subprocess.run([self.binary_dm_sched, '-h'], stderr=subprocess.PIPE, check=True, universal_newlines=True)
    findall = re.findall(r'../bin/dm-sched', process.stderr)
    self.assertTrue(len(findall) > 0, f'Called dm-sched: {re.findall("Usage:.*", process.stderr)}')
    self.assertEqual(findall[0], '../bin/dm-sched', f'Called dm-sched: {re.findall("Usage:.*", process.stderr)}')

  def test_environment_libcarpedm(self):
    """This test checks, which libcarpedm.so is used.
    The test passes, if '../lib/libcarpedm.so' is used (the shared object from the build environment).
    The test fails, if no libcarpedm.so is found or some other shared object is used.
    If dm-cmd is not found (which is needed for the ldd call), the failure is a CalledProcessError and the output contains:
    "subprocess.CalledProcessError: Command '['which', '../bin/dm-cmd']' returned non-zero exit status 1."
    If some other libcarpedm.so as the local build is used, the test fails with an AssertionError and the output contains:
    "Called libcarpedm: ['libcarpedm.so => /usr/local/lib/libcarpedm.so (0x00007f89e2e10000)']"
    or
    "Called libcarpedm: ['libcarpedm.so => not found']"
    """
    processWhich = subprocess.run(['which', self.binary_dm_cmd], stdout=subprocess.PIPE, check=True, universal_newlines=True)
    processLdd = subprocess.run(['ldd', processWhich.stdout[:-1]], stdout=subprocess.PIPE, check=True, universal_newlines=True)
    processGrep = subprocess.run(['grep' , 'libcarpedm'], input=processLdd.stdout, stdout=subprocess.PIPE, check=True, universal_newlines=True)
    self.assertTrue('../lib/libcarpedm.so' in processGrep.stdout, f'Called libcarpedm: {re.findall("libcarpedm.*", processLdd.stdout)}')
