#!/usr/bin/env python3
import subprocess
import sys
import re

args = ['git', 'log', '--pretty=format:%x00%H%x01%h%x01%P%x01%d%x00'] + sys.argv[1:]

rows = subprocess.check_output(args).split(b'\x00')[1::2]

seen_commits = set([row.split(b'\x01')[0].decode() for row in rows])

print("digraph G {")
for row in rows:
    columns = row.split(b'\x01')
    commit = columns[0].decode()
    label = columns[1].decode()
    parents = columns[2].decode().split(' ')
    tag = columns[3].decode()
    print('"{}" [label="{} {}"];'.format(commit, label, tag))
    for parent in parents:
        if parent == "":
            continue
        if not parent in seen_commits:
            continue
        print('"{}" -> "{}"'.format(commit, parent))
print("}")

