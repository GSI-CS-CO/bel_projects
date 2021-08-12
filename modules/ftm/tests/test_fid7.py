import dm_testbench
import csv

"""
Start a pattern and check with saft-ctl snoop if FID 7 occurs.
When FID 7 occurs, the test failed.
"""
class Fid7(dm_testbench.DmTestbench):
  def test_fid7(self):
    self.startPattern('fid.dot')
    file_name = 'snoop_protocol.csv'
    self.snoopToCsv(file_name, 2)
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
