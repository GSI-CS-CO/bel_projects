#!/bin/bash
dev=$1
fpga_type=$2
vendor_id="0x00000651"
a5_device_id="0x7e3d5e25"
a5_ext_1w_id="0x28"
a10_device_id="0xa1075000"
a10_ext_1w_id="0x42"
deg_offset="0x4"
device_id="NULL"
ext_1w_id="NULL"
addr="NULL"
addr_deg="NULL"
uptime="NULL"
temp_int="NULL"
temp_ext="NULL"
plot_started=0
gnuplot_pid=0
sleep_time_sec=10
plot_files=("gnuplot_int.pipe" "gnuplot_ext.pipe")

function sample_data()
{
  # Get time
  uptime=$(eb-mon $dev -v | grep "FPGA uptime" | awk {'print $4'})
  # Get temperatures
  temp_int=$(printf "%d\n" 0x$(eb-read $dev $addr_deg/4))
  # Convert value is necessary, magic values taken from: https://www.intel.com/content/www/us/en/docs/programmable/683585/current/transfer-function-for-internal-tsd.html
  if [ $fpga_type == a10 ]; then
    temp_int_dec=$(printf "%d" "$temp_int") # Hex to dec
    temp_int=$(echo "scale=2; ((693 * $temp_int_dec) / 1024)-265" | bc)
  fi
  echo "$uptime $temp_int" >> gnuplot_int.pipe
  temp_ext=$(eb-mon $dev -w0 -t0 -f $ext_1w_id)
  # Sometimes eb-mon will accidentally get a value like -0.0625 (less than zero). This is a bug, so we ignore this value.
  if [ $(echo "$temp_ext > 0" | bc) -eq 1 ]; then
    echo "$uptime $temp_ext" >> gnuplot_ext.pipe
  fi
}

function check_arguments()
{
  # check number of arguments
  if [ $# -eq 2 ]; then
    echo "Info: Checking device $dev ..."
  else
    echo "Error: Please provide a device name and a FPGA type!"
    echo "Example usage #1: $0 dev/ttyUSB0 a5"
    echo "Example usage #2: $0 tcp/scuxl1001.acc.gsi.de a10"
    exit 1
  fi
  # Check if device is available
  if [[ "${dev:0:3}" == "dev" ]]; then
    test -e /$dev
    if [ $? -ne 0 ]; then
      echo "Error: Device $dev does not exist!"
      exit 1
    fi
  else
    eb-ls $dev 2>&1 >> /dev/null
  fi
  # Check if FPGA type is supported
  if [ $fpga_type == a5 ]; then
    echo "Info: Using ArriaV profile ($fpga_type) ..."
    device_id=$a5_device_id
    ext_1w_id=$a5_ext_1w_id
  elif [ $fpga_type == a10 ]; then
    echo "Info: Using Arria10 profile ($fpga_type) ..."
    device_id=$a10_device_id
    ext_1w_id=$a10_ext_1w_id
  else
    echo "Error: FPGA type unknown!"
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
    if [ $fpga_type == a5 ]; then
      addr_deg=$(( $addr + deg_offset ))
    else
      addr_deg=$addr
    fi
  fi
}

function clean_up_log_files()
{
  # Clean up log files
  for file in "${plot_files[@]}"; do
    if [ -f $file ]; then
      rm $file
    fi
    touch $file
  done
}

# Trap SIGINT signal (Ctrl+C)
trap 'break' SIGINT

# Prepare everything
check_arguments $@
get_sensor_address
clean_up_log_files

# Read time and temperature
title="Temperature Measurements $dev - ($fpga_type)"
while true; do
  sample_data
  if [ $plot_started -eq 0 ]; then
    #gnuplot -p -e "set title \"$title\"; set autoscale xfix; set key top left box; set grid; set xlabel \"FPGA Uptime [Decimal Hours]\"; set ylabel \"Temperature [Degree Celsius]\"; set yrange [0:140]; plot \"gnuplot_int.pipe\" lt 2 lw 2 linecolor rgb \"orange\" title \"FPGA (Internal) Temperature\", \"gnuplot_ext.pipe\" lt 2 lw 2 linecolor rgb \"blue\" title \"Board (External) Temperature\"; while (1) { pause 1; replot; };" 2>>/dev/null &
gnuplot -p -e "set title \"$title\"; set autoscale xfix; set key top left box; set grid; set xlabel \"FPGA Uptime [Decimal Hours]\"; set ylabel \"Temperature [Degree Celsius]\"; set yrange [0:140]; plot \"gnuplot_int.pipe\" with linespoints lt 2 lw 2 pt 2 ps 1 linecolor rgb \"orange\" title \"FPGA (Internal) Temperature\", \"gnuplot_ext.pipe\" with linespoints lt 2 lw 2 pt 2 ps 1 linecolor rgb \"blue\" title \"Board (External) Temperature\"; while (1) { pause 1; replot; };" 2>>/dev/null &    gnuplot_pid=$!
    plot_started=1
    echo "Info: Press Ctrl+C to end the script, then close gnuplot ($gnuplot_pid) ..."
  fi
  sleep $sleep_time_sec
done
kill $gnuplot_pid
echo "Info: Finished sampling ..."
