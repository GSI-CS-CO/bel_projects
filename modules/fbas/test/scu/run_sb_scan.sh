#!/bin/sh

# Open another shell to see the debug output.

# Invoke 'eb-info -w dev/wbm0' to verify if any firmware runs in LM32

IMG=sb_scan.scucontrol.bin
DEV=dev/wbm0
SHM=0x20140508
CMD_PROBE_SB_DIOB=0x20   # probe DIOB slave card on SCU bus

# load FW
eb-fwload $DEV u 0 $IMG
sleep 1

# transit to 'configure' state
eb-write $DEV $SHM/4 0x1
sleep 1

# transit to 'opready' state
eb-write $DEV $SHM/4 0x2
sleep 1

# probe DIOB on the SCU bus
eb-write $DEV $SHM/4 $CMD_PROBE_SB_DIOB
