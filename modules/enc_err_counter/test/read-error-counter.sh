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

function read_enc_err_counter()
{
  if [ $interface -eq 1 ]
  then
    counterAddr=$addr
    overflowAddr=$(($addr+4))
  else
    counterAddr=$(($addr+8))    
    overflowAddr=$(($addr+12))
  fi
  counter=$(eb-read $dev $counterAddr/4)
  overflowFlag=$(eb-read $dev $overflowAddr/4)
  echo "Phy#1 counter: $counter overflow flag: $overflowFlag"  
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
read_enc_err_counter

exit 0
