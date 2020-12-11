#!/usr/bin/python
import sys
import re
import stat
import os

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
def extractScript(commands_history_file):
    script_file = 'commands.sh'
    if 'GeneratorCommandsHistory' in commands_history_file:
        script_file = f'{commands_history_file[9:-4]}.sh'
    print(f'Rewrite command history file {commands_history_file} to script {script_file}.')
    createFolder(script_file[:-3])
    prefix = script_file[:-3] + '/graph-entry'
    print(f'Schedules written to {prefix}-n.dot, where n is the entry number.')
    # Read comand history file
    file_h = open(commands_history_file, 'r')
    lines = file_h.readlines()
    pattern = re.compile('# log entry ([0-9]+) ------------')
    entry_no = -1
    dot_file_name = ''
    collect_lines = False
    graph_lines = []
    script = open(script_file, 'w')
    script.write(f'#! /bin/bash\n\n')
    script.write(f'# Usage: {script_file} [options]\n')
    script.write(f'#        where options are used for dm-cmd and dm-sched\n')
    script.write(f'#        Data Master name is expected in $DM.\n')
    script.write(f'# Example: ./commands.sh -v\n')
    script.write(f'OPTIONS=$1\n')
    for i in range(len(lines)):
        match = pattern.match(lines[i])
        if match:
            if int(entry_no) > -1 and len(graph_lines) > 0:
                with open(dot_file_name, 'w') as writer:
                    writer.write(''.join(graph_lines[1:]))
            entry_no = match.group(1)
            dot_file_name = f'{prefix}-{entry_no}.dot'
            collect_lines = False
            graph_lines = []
            print(f'Entry: {entry_no} {lines[i+2][:-1]}')
        if 'schedule remove' in lines[i]:
            collect_lines = True
            script.write(f'# entry {entry_no}\n')
            script.write(f'dm-sched $DM $OPTIONS remove {dot_file_name}\n')
        if collect_lines:
            graph_lines.append(lines[i])
    print(f'Lines: {len(lines)}.')
    st = os.stat(script_file)
    os.chmod(script_file, st.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

if __name__ == '__main__':
    if len(sys.argv) > 1:
        extractScript(sys.argv[1])
    else:
        print(f'File name for command history missing')

