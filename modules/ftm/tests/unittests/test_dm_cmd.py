import unittest
import subprocess
import sys

global test_binary
global data_master

"""
Class collects unit tests for the command line of dm-cmd.
First section: all commands which need a target name. Test case names: test_<command>_missing, test_<command>.
"""
class TestDmCmd(unittest.TestCase):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.binary = test_binary
    self.data_master = data_master

  def t1est_print_args(self):
    print(f'Binary: {self.binary}, data master: {self.data_master}')

  def targetName_missing(self, command):
    """
    Common method for test cases with missing target name. 
    Start dm-cmd with the command and check the output on stderr.
    """
    # pass cmd and args to the function
    process = subprocess.Popen([self.binary, self.data_master, command], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    lines = stderr.decode('utf-8').splitlines()
#    print(f'Lines:\n{lines}')
    test_result = False
    for line in lines:
      if 'Target node is NULL, target missing.' in line:
        test_result = True
    self.assertTrue(test_result, f'stderr: {lines}')
    self.assertEqual(len(lines), 1)

  def test_abswait_missing(self):
    self.targetName_missing('abswait')

  def test_asyncclear_missing(self):
    self.targetName_missing('asyncclear')

  def test_flow_missing(self):
    self.targetName_missing('flow')

  def test_flush_missing(self):
    self.targetName_missing('flush')

  def test_lock_missing(self):
    self.targetName_missing('lock')

  def test_noop_missing(self):
    self.targetName_missing('noop')

  def test_queue_missing(self):
    self.targetName_missing('queue')

  def test_rawqueue_missing(self):
    self.targetName_missing('rawqueue')

  def test_relwait_missing(self):
    self.targetName_missing('relwait')

  def test_staticflush_missing(self):
    self.targetName_missing('staticflush')

  def test_switch_missing(self):
    self.targetName_missing('switch')

  def test_unlock_missing(self):
    self.targetName_missing('unlock')

  def targetName(self, command, count_out=0, count_err=0, options=''):
    """
    Common method for commands with target name.
    Start dm-cmd with the command and check the output on stdout and stderr.
    The checks check only the number of lines of the output, not the content.
    """
    process = subprocess.Popen([self.binary, self.data_master, command, 'B_PPS', options], stderr=subprocess.PIPE, stdout=subprocess.PIPE)   # pass cmd and args to the function
    stdout, stderr = process.communicate()   # get command output and error
    lines = stdout.decode('utf-8').splitlines()
#    print(f'Lines:\n{lines}')
    self.assertEqual(len(lines), count_out, f'stdout, Number of lines {len(lines)}')
    lines = stderr.decode('utf-8').splitlines()
#    print(f'Lines:\n{lines}')
    self.assertEqual(len(lines), count_err, f'stderr, Number of lines {len(lines)}')

  def test_abswait(self):
    self.targetName('abswait', options='100')

  def test_asyncclear(self):
    self.targetName('asyncclear')

  def test_flow(self):
    self.targetName('flow', count_err=1, options='x')

  def test_flush(self):
    self.targetName('flush', count_err=1)

  def test_lock(self):
    self.targetName('lock')

  def test_noop(self):
    self.targetName('noop')

  def test_queue(self):
    self.targetName('queue', count_out=9)

  def test_rawqueue(self):
    self.targetName('rawqueue', count_out=30)

  def test_relwait(self):
    self.targetName('relwait', options='100')

  def test_staticflush(self):
    self.targetName('staticflush', count_err=1)

  def test_switch(self):
    self.targetName('switch', count_err=1, options='x')

  def test_unlock(self):
    self.targetName('unlock')

if __name__ == '__main__':
  if len(sys.argv) > 2:
#    print(f"Arguments: {sys.argv}")
    data_master = sys.argv.pop()
    test_binary = sys.argv.pop()
#    print(f"Arguments: {sys.argv}, {len(sys.argv)}")
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)
