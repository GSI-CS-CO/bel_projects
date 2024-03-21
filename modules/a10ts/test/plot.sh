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
sleep_time_sec=10

function sample_data()
{
  uptime=$(eb-mon $dev -v | grep "FPGA uptime" | awk {'print $4'})
  temp=$(printf "%d\n" 0x$(eb-read $dev $addr_deg/4))
  echo "$uptime $temp" >> gnuplot.pipe
}

function check_arguments()
{
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
}

function get_sensor_address()
{
  # Check address
  addr=$(eb-find $dev $vendor_id $device_id)
  if [ $? -ne 0 ]; then
    echo "Error: Can't find device!"
    exit 1
  else
    echo "Info: Found device at $addr ..."
    # This line was added to support the ArriaV sensor
    addr_deg=$(( $addr + deg_offset ))
  fi
}

# Trap SIGINT signal (Ctrl+C)
trap 'break' SIGINT

# Prepare everything
check_arguments $@
get_sensor_address

# Clean up
if [ -f gnuplot.pipe ]; then
  rm gnuplot.pipe
fi
touch gnuplot.pipe

# Read time and temperature
while true
do
  sample_data
  if [ $plot_started -eq 0 ]; then
    gnuplot -p -e 'set title "FPGA Temperature"; set grid; set xlabel "FPGA Uptime [Decimal Hours]"; set ylabel "Temperature [Degree Celsius]"; set yrange [0:100]; plot "gnuplot.pipe" smooth bezier lt 2 lw 2 linecolor rgb "blue" title "Temperature"; while (1) { pause 1; replot; };' 2>>/dev/null &
    gnuplot_pid=$!
    plot_started=1
    echo "Info: Press Ctrl+C to end the script, then close gnuplot ($gnuplot_pid) ..."
  fi
  sleep $sleep_time_sec
done
kill $gnuplot_pid
