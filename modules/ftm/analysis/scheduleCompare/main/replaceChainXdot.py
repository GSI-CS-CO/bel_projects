#! /usr/bin/python3
import subprocess
import sys

index = len(sys.argv) - 1
process1 = subprocess.Popen(["replaceChain", "-s"], stdout=subprocess.PIPE, text=True)
process2 = subprocess.Popen(["dot", '-Txdot'], stdin=process1.stdout, stdout=subprocess.PIPE, text=True)
output, error = process2.communicate()
print(output)
