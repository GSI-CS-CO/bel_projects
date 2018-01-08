#!/bin/sh
# starts and configures the firmware (lm32) and software (host) of wr-mil gateway for ESR operation

# set -x

# reset lm32 firmware
echo "reset lm32"
wr-mil-gw-ctl dev/wbm0 -r 
sleep 1

# load firmware
echo "load firmware"
eb-fwload dev/wbm0 u1 0 wr_mil.bin

# destoy all unowned conditions
saft-ecpu-ctl baseboard -x
# create new condition for all MIL events ('0' in first four bits of EVTNO)
#      GROUP-ID for ESR  --------|||
#                                VVV
saft-ecpu-ctl baseboard -g -z -c 0x1154000000000000 20 100000 4 -d 
sleep 1

# configure as ESR gateway and run it
wr-mil-gw-ctl dev/wbm0 -e

# done
echo "wr-mil-gw started in ESR mode"
