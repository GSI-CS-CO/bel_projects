import sys
import time
import subprocess
import signal
import csv
import pathlib
import dm_testbench

"""
Start a pattern and check with saft-ctl snoop that BPC start flag works.

Required argument:  device of data master.
"""
if len(sys.argv) > 1:
    file_test_pattern = 'bpcStart.dot'
    dm_testbench.startpattern(sys.argv[1], file_test_pattern)
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
            test1 = row[10] != '0x4'
            test2 = row[10] != '0xc'
#            print(f'Row: {row}, row[10]: {row[10]}, {test1}, {test2}')
            if test1 and test2:
                test_result = False
                break
        print(f'Snoop: processed {line_count} lines, test result is {test_result}.')
    file_to_rem = pathlib.Path(file_name)
    file_to_rem.unlink()
    if test_result:
        file_name = 'd1.dot'
        process = subprocess.Popen(['dm-sched', sys.argv[1], '-o', file_name], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        stdout, stderr = process.communicate()
        with open(file_name, 'r') as reader:
            lines = reader.readlines()
        file_to_rem = pathlib.Path(file_name)
        file_to_rem.unlink()
#            print(lines)
        if len(lines) >= 10:
            test_result = 'bpcstart="1"' in lines[4] and 'bpcstart="1"' in lines[5]
        else:
            test_result = False
            print(f'dm-sched: output too short ({len(lines)} lines), result: {test_result}')
    print(f'Test BPC start, result {test_result}')
else:
    print('Required argument (data master device) missing.')
