import sys
import time
import subprocess
import signal
import csv
import pathlib
import dm_testbench

"""
Start a pattern and check with saft-ctl snoop if FID 7 occurs.
When FID 7 occurs, the test failed.

Required argument:  device of data master.
"""
if len(sys.argv) > 1:
    file_test_pattern = 'fid.dot'
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
            if row[4] == '0x7':
                test_result = False
                break
        print(f'Processed {line_count} lines, test result is {test_result}.')
    file_to_rem = pathlib.Path(file_name)
    file_to_rem.unlink()
else:
    print('Required argument (data master device) missing.')
