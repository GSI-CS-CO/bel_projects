#!/bin/sh

# Scan the SCU bus by using the dedicated LM32 FW binary.
# The scan result is output to LM32 UART, which can be
# snooped by 'eb-console'.

IMG=sb_scan.scucontrol.bin
DEV=dev/wbm0
SHM=0x20140508
CMD_PROBE_SB_DIOB=0x20   # probe DIOB slave card on SCU bus

# check FW image file
filepath="$HOME/$IMG"
if [ ! -f "$filepath" ]; then
    echo "Error: $filepath not found. Exit!"
    exit 1
fi

# info
echo "To check the scan result, launch 'eb-console $DEV' in another shell."

# load FW
eb-fwload $DEV u 0 $filepath
sleep 1

# transit to 'configure' state
eb-write $DEV $SHM/4 0x1
sleep 1

# transit to 'opready' state
eb-write $DEV $SHM/4 0x2
sleep 1

# probe DIOB on the SCU bus
eb-write $DEV $SHM/4 $CMD_PROBE_SB_DIOB
