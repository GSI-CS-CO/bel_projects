#!/bin/bash
dev=$1
vendor_id="0x00000651"
device_id="0x11223344"
base_addr="0x7fff0000"

# check if device is available
test -e /$dev
if [ $? -ne 0 ]; then
  echo "Error: Device $dev does not exist!"
  exit 1
fi

# check address
addr_check=$(eb-find $dev $vendor_id $device_id)
if [[ "$addr_check" != "$base_addr" ]]; then
  echo "Error: Address does not match!"
  echo "Expected: $base_addr"
  echo "Found:    $addr_check"
  exit 2
fi

# check single writes and reads
echo "Testing single access mode..."
echo "=================================================="
counter=1
hex_offset=0
hex_addr=0
while [ $counter -le 8 ]; do
  echo "Iteration: $counter -> 0x$hex_offset (offset) -> 0x$hex_addr (destination)"
  dword_set="0xffffffff"
  dword_get="0x00000000"
  hex_addr=$(echo $(($base_addr + $hex_offset)))
  hex_addr=$(echo 16o${hex_addr}p |dc)

  # get random hex value, cut off spaces and translate upper to lower chars
  dword_set=$(hexdump -n 4 -e '4/4 "%08X" 1 "\n"' /dev/urandom | awk '{$1=$1};1' | tr '[:upper:]' '[:lower:]')
  eb-write $dev 0x$hex_addr/4 0x$dword_set
  echo "Write: 0x$dword_set to   0x$hex_addr/4 "
  dword_get=$(eb-read $dev 0x$hex_addr/4)
  echo "Read:  0x$dword_get from 0x$hex_addr/4 "

  # compare write and read
  if [[ "$dword_get" != "$dword_set" ]]; then
    echo "Write/read does not match!"
    exit 3
  else
    echo "Match!"
  fi

  # loop control
  hex_offset="$(($counter * 4))"
  ((counter++))
done
echo ""

# check streaming interface
echo "Testing streaming access mode..."
echo "=================================================="
dd if=/dev/urandom of=random_file_set bs=32 count=1
eb-put $dev $base_addr random_file_set
eb-get $dev $base_addr/32 random_file_get
diff random_file_set random_file_get
if [ $? -ne 0 ]; then
  echo "Error: Streaming mode borken!"
  echo "Send:"
  hexdump -C random_file_set
  echo "Received:"
  hexdump -C random_file_get
  exit 4
fi
echo ""

# done
echo "Unit works fine!"
exit 0
