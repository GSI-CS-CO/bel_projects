#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import subprocess
import sys, getopt

# Function perform_system_call(...)
# --------------------------------------------------------------------------------
def perform_system_call(command):
  try:
    result = subprocess.check_output(command, stderr=subprocess.STDOUT, shell=True)
    result = result.replace('\n', ' ').replace('\r', '')
    command = command.replace('\n', ' ').replace('\r', '')
    #print command
    #print result
    #print "============================================================"
    return 0  
  except subprocess.CalledProcessError, ex: # Error code <> 0 
    print "Subprocess Error:"
    print ex.cmd
    print ex.message
    print ex.returncode
    print ex.output # Contains stdout and stderr together   
    return 1

# Function show_help(...)
# --------------------------------------------------------------------------------
def show_help():
  print "============================================================"
  print "Usage:   -d <device> -a <address> -s <size>"
  print "Example: -d udp/192.168.191.79 -a 0x310 -s 512"
  print "Arguments:"
  print "  -d Device"
  print "  -a Address (address of rx fifo)"
  print "  -s Sample size"
  print "============================================================"

# Function get_data_from_device(...)
# --------------------------------------------------------------------------------
def get_data_from_device():
  perform_system_call("touch audio.log")
  perform_system_call("rm audio.log")
  perform_system_call("touch audio.log")
  for i in range(0, (int(size)-1)):
    # Create new file with the format: ID/Time, Value
    command = "{0} {1} {2} {3} {4}/4 {5}".format("echo -n", i, ", 0x | tee -a audio.log; eb-read", device, address, "2>&1 | tee -a audio.log") 
    perform_system_call(command)

# Function get_data_from_file(...)
# --------------------------------------------------------------------------------
def get_data_from_file():
  global time
  global value
  # Data is provided at hexadecimal format
  data = np.genfromtxt('audio.log',delimiter=',', dtype = '|S16')
  time = [row[0] for row in data]
  value = [row[1] for row in data]
  # Convert from hexadecimal to decimal
  for i in range(0, len(value)):
    value[i] = int(value[i], 16)

# Function show_data(...)
# --------------------------------------------------------------------------------
def show_data():
  print "Time(s):"
  print time
  print "Value(s):"
  print value

# Function show_plot(...)
# --------------------------------------------------------------------------------
def show_plot():
  fig = plt.figure()
  ax = fig.add_subplot(111, axisbg = 'w')
  ax.plot(time,value,'b',lw=1.3, linestyle='-', label="Connected Points")
  ax.plot(time,value,'r',lw=1.3, marker='o', linestyle='None', label="Single Points")
  plt.xlabel('Sample Number')
  plt.ylabel('ADC Value')
  plt.title('Audio Sample')
  plt.legend()
  plt.grid()
  plt.show()

# Function main(...)
# --------------------------------------------------------------------------------
def main(argv):
  # Helpers
  global device
  global address
  global size
  device = ''
  address = ''
  size = ''
  
  # Welcome message
  print "Etherbone Audio Plotter started ..."
  
  # Check if all necessary arguments are present
  try:
    opts, args = getopt.getopt(argv,"cd:a:s:",["dev=","adr=","siz="])
  except getopt.GetoptError:
    show_help()
    sys.exit(1)
  
  # Get values from arguments
  for opt, arg in opts:
    if opt in ("-d", "--dev"):
      device = arg
    elif opt in ("-a", "--adr"):
      address = arg
    elif opt in ("-s", "--siz"):
      size = arg
  
  # Get data and plot it
  print "Getting data from device ..."
  get_data_from_device()
  get_data_from_file()
  print "Creating plot now ..."
  show_plot()

if __name__ == "__main__":
   main(sys.argv[1:])
