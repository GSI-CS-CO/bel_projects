#!/bin/sh

IMG=sb_scanner.bin
DEV=dev/wbm0
SHM=0x20140508/4

# load FW
eb-fwload $DEV u 0 $IMG
sleep 1

# transit to 'configure' state
eb-write $DEV $SHM 0x1
sleep 1

# transit to 'opready' state
eb-write $DEV $SHM 0x2
sleep 1

# probe DIOB on the SCU bus
eb-write $DEV $SHM 0x20
