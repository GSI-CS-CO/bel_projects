#! /usr/bin/env python3
import sys
import re
import stat
import os
from datetime import datetime as dt
import argparse

def createFolder(folder):
    try:
        if not os.path.exists(folder):
            os.makedirs(folder)
    except OSError:
        print (f'Error creating folder "{folder}".')

'''
Take a generator command history file and extract all commands and the input dot files.
Output: 1. bash script with similar name as history file
        2. folder with extracted dot files
'''
def extractScript(commands_history_file, dtBegin, dtEnd, verbose):
    script_file = 'commands.sh'
    if 'GeneratorCommandsHistory' in commands_history_file:
        script_file = f'{commands_history_file[9:-4]}.sh'
    print(f'Rewrite command history file {commands_history_file} to script {script_file}.')
    createFolder(script_file[:-3])
    prefix = script_file[:-3] + '/graph-entry'
    print(f'Schedules written to {prefix}-n.dot, where n is the entry number.')
    print(f'First timestamp: {dtBegin}, last timestamp: {dtEnd}')
    pattern = re.compile('# log entry ([0-9]+) ------------')
    # Match something like '# Fri_Feb__5_13.36.22_2021'
    timestampPattern = re.compile('# ([FMSTW][a-z]{2}_[A-Z][a-z]{2}_+[0-9]{1,2}_[0-9]{1,2}.[0-9]{1,2}.[0-9]{1,2}_[0-9]{4})')
    entry_no = -1
    dot_file_name = ''
    collect_lines = False
    # recordEntries is set to True, iff the time stamp of the entry is between Begin and End.
    recordEntries = False
    # find the first and the last entry
    dtFirstEntry = dt(2100,1,1)
    dtLastEntry = dt(1970,1,1)
    # collector for the graph written to a dot file.
    graph_lines = []
    countDotFiles = 0
    # Read comand history file
    lines = []
    with open(commands_history_file, 'r') as file_h:
      lines = file_h.readlines()
    lines.append('# log entry 999999 ------------')
    with open(script_file, 'w') as script:
      # Write header of script file
      script.write(f'#! /bin/bash\n\n')
      script.write(f'# Usage: {script_file} [options]\n')
      script.write(f'#        where options are used for dm-cmd and dm-sched\n')
      script.write(f'#        Data Master name is expected in $DM.\n')
      script.write(f'# Example: ./commands.sh -v\n')
      script.write(f'OPTIONS=$1\n')
      for i in range(len(lines)):
        match = pattern.match(lines[i])
        if match:
            # write graph_lines to dot file only if there is a graph.
            # improvement: check for 'digraph' in the first line.
            if recordEntries and int(entry_no) > -1 and len(graph_lines) > 2 and len(graph_lines[1]) > 6:
              with open(dot_file_name, 'w') as writer:
                writer.write(''.join(graph_lines[1:]))
                countDotFiles += 1
            # extract the entry number.
            entry_no = match.group(1)
            if len(entry_no) < 6:
              entry_no = '000000'[0:6-len(entry_no)] + entry_no
            dot_file_name = f'{prefix}-{entry_no}.dot'
            timestampMatch = timestampPattern.match("")
            if i+1 < len(lines):
              timestampMatch = timestampPattern.match(lines[i+1])
            if timestampMatch:
              testdate = timestampMatch.group(1).replace('__', '_0')
              dtTestdate = dt.strptime(testdate, "%a_%b_%d_%H.%M.%S_%Y")
              if dtFirstEntry > dtTestdate:
                dtFirstEntry = dtTestdate
              if dtLastEntry < dtTestdate:
                dtLastEntry = dtTestdate
              recordEntries = (dtBegin < dtTestdate) and (dtTestdate < dtEnd)
            collect_lines = False
            graph_lines = []
            if verbose and i+2 < len(lines):
              print(f'Entry: {entry_no} {timestampMatch.group(1)} {lines[i+2][:-1]}')
        if recordEntries:
          if 'schedule clear' in lines[i]:
            collect_lines = False
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'dm-cmd $DM $OPTIONS halt\n')
            script.write(f'dm-sched $DM $OPTIONS clear\n')
          elif 'schedule add' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'dm-sched $DM $OPTIONS add {dot_file_name}\n')
          elif 'schedule keep' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'dm-sched $DM $OPTIONS keep {dot_file_name}\n')
          elif 'schedule remove' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'dm-sched $DM $OPTIONS remove {dot_file_name}\n')
          elif 'schedule dump' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'dm-sched $DM $OPTIONS status -o {dot_file_name}\n')
          elif 'table check result:' in lines[i]:
            collect_lines = False
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}, {lines[i]}')
          elif 'command execute' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'dm-cmd $DM $OPTIONS -i {dot_file_name}\n')
          elif 'is safe to remove' in lines[i]:
            collect_lines = False
            try:
              pattern_name = re.search('is safe to remove: (.+)$', lines[i]).group(1)
              script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
              script.write(f'dm-cmd $DM $OPTIONS chkrem {pattern_name}\n')
            except AttributeError:
              print(f'Pattern name not found: {lines[i]}, entry {entry_no}')
          elif 'result' in lines[i]:
            collect_lines = False
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}, {lines[i]}')
          elif 'queue status for pattern' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'status report' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'static flush pattern' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'queue report for pattern' in lines[i]:
            collect_lines = False
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}, lines[i]')
          elif 'set maintenance mode to true' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'set maintenance mode to false' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'set force schedule actions to true' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'set force schedule actions to false' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'set optimizeSafeToRemove to true' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
          elif 'set optimizeSafeToRemove to false' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no} {dtTestdate}\necho entry {entry_no} {dtTestdate}\n')
            script.write(f'# dm- $DM $OPTIONS  {dot_file_name}\n')
        if collect_lines:
          graph_lines.append(lines[i])
    print(f'Lines: {len(lines)}, dot files: {countDotFiles}, first entry: {dtFirstEntry}, last entry: {dtLastEntry}.')
    st = os.stat(script_file)
    os.chmod(script_file, st.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

def convertTimestamp(dtString):
  """
  Parse a timestamp (date or date and time).
  """
  try:
    if len(dtString) > 19:
      # parse 2021-03-12 12:20:07.6
      # parse 2021-03-12 12:20:07.000006
      # parse 2021-03-12 12:20:07.691156
      return dt.strptime(dtString, "%Y-%m-%d %H:%M:%S.%f")
    if len(dtString) > 17:
      # parse 2021-03-12 12:20:07
      return dt.strptime(dtString, "%Y-%m-%d %H:%M:%S")
    if len(dtString) > 13:
      # parse 2021-03-12 11:17
      return dt.strptime(dtString, "%Y-%m-%d %H:%M")
    if len(dtString) > 11:
      # parse 2021-03-12 11
      return dt.strptime(dtString, "%Y-%m-%d %H")
    if len(dtString) > 9:
      # parse 2021-03-12
      return dt.strptime(dtString, "%Y-%m-%d")
    else:
      return dt.now()
  except ValueError as v:
    print(v)
    return dt(1, 1, 1)

def main(argv):
  parser = argparse.ArgumentParser("CommandHistory reads datamaster log files and generates a script and a series of dot files for the script.",
                                    epilog="For questions and bugs contact Martin Skorsky (m.skorsky@gsi.de).")
  parser.add_argument('--version', action='version', version='%(prog)s 1.0')
  parser.add_argument('--verbose', '-v', help='print each log entry')
  parser.add_argument('log_file_name', help='the name of the datamaster log file')
  parser.add_argument('begin_date', help='the first date to log (year, month, day all zero-padded)')
  parser.add_argument('begin_time', help='the time part of the first date to log')
  parser.add_argument('end_date', help='the last date to log (year, month, day all zero-padded)')
  parser.add_argument('end_time', help='the time part of the last date to log')
  args = parser.parse_args(argv)
  extractScript(args.log_file_name, convertTimestamp(args.begin_date + " " + args.begin_time), convertTimestamp(args.end_date + " " + args.end_time), args.verbose)
    
if __name__ == '__main__':
  main(argv=sys.argv[1:])
