
import csv
import dm_testbench

"""
Start a pattern and check with saft-ctl snoop that BPC start flag works.
"""
class BpcStart(dm_testbench.DmTestbench):

  def test_bpcstart(self):
    self.startPattern('bpcStart.dot')
    file_name = 'snoop_protocol.csv'
    self.snoopToCsv(file_name, duration=2)
    # Read this file snoop_protocol.csv as csv
    test_result = True
    with open(file_name) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=' ')
        line_count = 0
        for row in csv_reader:
            line_count += 1
            test1 = row[10] != '0x4'
            test2 = row[10] != '0xc'
#            print(f'Row: {row}, row[10]: {row[10]}, {test1}, {test2}')
            if test1 and test2:
                test_result = False
                break
        if line_count == 0:
          test_result = False
    self.deleteFile(file_name)
    self.assertTrue(test_result, f'Snoop: processed {line_count} lines, test result is {test_result}.   ')
    file_name = 'd1.dot'
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, '-o', file_name])
    with open(file_name, 'r') as reader:
        lines = reader.readlines()
    self.deleteFile(file_name)
#            print(lines)
    if len(lines) >= 10:
        test_result = 'bpcstart="1"' in lines[4] and 'bpcstart="1"' in lines[5]
    else:
        test_result = False
    self.assertTrue(test_result, f'dm-sched: output too short ({len(lines)} lines), result: {test_result}')
