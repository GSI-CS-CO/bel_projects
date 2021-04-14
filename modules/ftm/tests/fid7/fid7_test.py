#! /usr/bin/env python3

import sys
import time
import subprocess
import signal
import csv
import pathlib
import dm_testbench
import unittest

"""
Start a pattern and check with saft-ctl snoop if FID 7 occurs.
When FID 7 occurs, the test failed.

Required argument:  device of data master.
"""
class Fid7(dm_testbench.DmTestbench):

  def setUp(self):
    """
    Set up for all test cases: store the arguments in class variables.
    """
    self.datamaster = test_datamaster

  def test_fid7(self):
    file_test_pattern = 'fid.dot'
    self.startPattern(self.datamaster, file_test_pattern)
    file_name = 'snoop_protocol.csv'
    process = subprocess.Popen(['saft-ctl', 'x', '-fvx', 'snoop', '0', '0', '0'], stderr=subprocess.PIPE, stdout=subprocess.PIPE)   # pass cmd and args to the function
    time.sleep(1) # adopt to pattern: how many messages a pattern produces in a second
    process.send_signal(signal.SIGINT)   # send Ctrl-C signal
    stdout, stderr = process.communicate()   # get command output and error
    # Write stdout to file snoop_protocol.csv
    with open(file_name, 'wb') as writer:
        writer.write(stdout)
    # Read this file snoop_protocol.csv as csv
    test_result = True
    with open(file_name) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=' ')
        line_count = 0
        for row in csv_reader:
            line_count += 1
            if row[4] == '0x7':
                test_result = False
                break
    self.deleteFile(file_name)
    self.assertTrue(test_result, f'Processed {line_count} lines, test result is {test_result}.')

if __name__ == '__main__':
  if len(sys.argv) > 1:
    test_datamaster = sys.argv.pop()
    unittest.main(verbosity=2)
  else:
    print("Required argument missing", sys.argv)
