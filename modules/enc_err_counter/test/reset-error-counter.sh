#!/bin/sh
dev=$1
interface=$2
addr="NULL"
vendor_id="0x00000651"
device_id="0x434e5452"

function get_sensor_address()
{
  # Check address
  addr=$(eb-find $dev $vendor_id $device_id)
  if [ $? -ne 0 ]; then
    echo "Error: Can't find device or module!"
    exit 1
  else
    echo "Info: Found device at $addr ..."
  fi
}

function reset_enc_err_counter()
{
  if [ $interface -eq 1 ]
  then
    counterAddr=$addr
  else
    counterAddr=$(($addr+8))    
  fi
  eb-write $dev $counterAddr/4 0x00000001
  sleep 1
  eb-write $dev $counterAddr/4 0x00000000
  echo "Reset carried out. Check counter for confirmation."
}

if [ -z "$1" ]
then
	echo "expecting non-optional argument: <device>"
	exit 1
fi

if [ -z "$2" ]
then
	interface=1
fi

get_sensor_address
reset_enc_err_counter

exit 0
