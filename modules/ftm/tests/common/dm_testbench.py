import dm
import subprocess

"""
dm_testbench collects functions to handle patterns for the data master testbench.
"""
def startpattern(data_master, pattern_file):
    """
    Connect to the given data master and load the pattern file (dot format).
    The data master is halted, cleared, and statistics is reset.
    Search for the first pattern in the data master with 'dm-sched' and start it.
    """
    print (f"Connect to device {data_master}, pattern file {pattern_file}")
    dmobj = dm.CarpeDM()
    dmobj.connect(data_master)
    dmobj.halt()
    dmobj.clear(True)
    dmobj.softwareReset(True)
    dmobj.addDotFile(pattern_file, False);
    dmobj.downloadDot(False)

    # run 'dm-sched data_master' as a sub process.
    process = subprocess.Popen(['dm-sched', data_master], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    # get command output and error
    stdout, stderr = process.communicate()
    lines = stdout.decode('utf-8').splitlines()
    pattern_name = 'x'
    for i in range(len(lines)):
      if lines[i] == 'Patterns:':
        pattern_name = lines[i+1]
        break
    # start the first pattern found in the data master.
    dmobj.startPattern(pattern_name, 0)
