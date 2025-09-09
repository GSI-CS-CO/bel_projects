#!/usr/bin/env bash

read_start_address=0x04004000
write_start_address=0x04004400
transfer_size=16

eb-put dev/ttyUSB0 $(printf "0x%X" "$read_start_address") addressRAM.bin
eb-write dev/ttyUSB0 0x0/4 $(printf "0x%X" "$transfer_size")
eb-write dev/ttyUSB0 0x4/4 $(printf "0x%X" "$read_start_address")
eb-write dev/ttyUSB0 0x8/4 $(printf "0x%X" "$write_start_address")

eb-write dev/ttyUSB0 0xc/4 0x1
sleep 1
eb-write dev/ttyUSB0 0xc/4 0x0
sleep 1
eb-write dev/ttyUSB0 0xc/4 0x1
sleep 1
eb-write dev/ttyUSB0 0xc/4 0x0

for ((i=0; i<transfer_size; i++));
do
    golden_address=$(($(printf "0x%X" "$read_start_address") + i*4))
    test_address=$(($(printf "0x%X" "$write_start_address") + i*4))
    golden=$(eb-read dev/ttyUSB0 $(printf "0x%X" "$golden_address")/4)
    test=$(eb-read dev/ttyUSB0 $(printf "0x%X" "$test_address")/4)
    if (( 0x$golden != 0x$test )); then
        printf "Address 0x%X not equal to 0x%X.\n" "$golden_address" "$test_address"
    fi
done