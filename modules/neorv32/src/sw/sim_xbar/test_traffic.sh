#! /bin/bash

while true; do
eb-put dev/ttyUSB0 0x4060100 smallRAM.bin
eb-get dev/ttyUSB0 0x4060100/256 result

sleep 0.1

cmp result smallRAM.bin
done

# while true; do
# eb-write dev/ttyUSB0 0x04060100/4 0x12345678
# done
