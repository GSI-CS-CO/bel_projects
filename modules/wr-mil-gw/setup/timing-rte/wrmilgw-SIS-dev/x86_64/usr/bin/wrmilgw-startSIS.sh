#!/bin/sh
# starts and configures the firmware (lm32) and software (host) of wr-mil gateway for SIS operation

# set -x

# reset lm32 firmware
echo "reset lm32"
wr-mil-gw-ctl dev/wbm0 -r 
sleep 1

# put all lm32 user cores in reset state
eb-write dev/wbm0 0x20000018/4 0xfffffffd
# load firmware
echo "load firmware"
eb-fwload dev/wbm0 u1 0 wr_mil.bin

# destoy all unowned conditions
saft-ecpu-ctl tr0 -x
# create new condition for all MIL events ('0' in first four bits of EVTNO)
#    GROUP-ID for SIS --------|||
#                             VVV
saft-ecpu-ctl tr0 -z -g -c 0x112c000000000000 20 100000 4 -d 

# trigger UTC timestamps on event 246 (BEGIN_CMD_EXEC)
#wr-mil-gw-ctl dev/wbm0 -t 246

# trigger UTC timestamps on event 255 ( EVT_COMMAND )
wr-mil-gw-ctl dev/wbm0 -t 255

# no delay between trigger event and UTC timestamp generataion
wr-mil-gw-ctl dev/wbm0 -u 0
wr-mil-gw-ctl dev/wbm0 -d 0

# configure as SIS gateway and run it
sleep 1
wr-mil-gw-ctl dev/wbm0 -s

# done
echo "wr-mil-gw started in SIS mode"
