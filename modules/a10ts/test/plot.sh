#!/bin/bash
dev=$1
vendor_id="0x00000651"
device_id="0x7e3d5e25"
addr="NULL"
addr_deg="NULL"
deg_offset="0x4"
uptime="NULL"
temp="NULL"
plot_started=0
gnuplot_pid=0
sleep_time_sec=30

# Trap SIGINT signal (Ctrl+C)
trap 'break' SIGINT

# check number of arguments
if [ $# -eq 1 ]; then
  echo "Info: Checking device $dev ..."
else
  echo "Error: Please provide a device name!"
  exit 1
fi

# Check if device is available
test -e /$dev
if [ $? -ne 0 ]; then
  echo "Error: Device $dev does not exist!"
  exit 1
fi

# Check address
addr=$(eb-find $dev $vendor_id $device_id)
if [ $? -ne 0 ]; then
  echo "Error: Can't find device!"
  exit 1
else
  echo "Info: Found device at $addr ..."
  addr_deg=$(( $addr + deg_offset ))
fi

# Clean up
if [ -f gnuplot.pipe ]; then
  rm gnuplot.pipe
fi
touch gnuplot.pipe

# Read time and temperature
while true
do
  uptime=$(eb-mon $dev -v | grep "FPGA uptime" | awk {'print $4'})
  temp=$(printf "%d\n" 0x$(eb-read $dev $addr_deg/4))
  echo "$uptime $temp" >> gnuplot.pipe
  sleep $sleep_time_sec
  if [ $plot_started -eq 0 ]; then
    gnuplot -p -e 'plot "gnuplot.pipe"; while (1) { pause 1; replot; };' 2>>/dev/null &
    gnuplot_pid=$!
    plot_started=1
    echo "Info: Press Ctrl+C to end the script, then close gnuplot ..."
  fi
done

kill $gnuplot_pid
