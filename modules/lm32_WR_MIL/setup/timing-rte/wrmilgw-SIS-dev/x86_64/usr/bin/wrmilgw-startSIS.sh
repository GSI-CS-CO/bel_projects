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
# create new conditions
saft-ecpu-ctl baseboard -z -c 0x0000020008000000 64 0 4 -d
saft-ecpu-ctl baseboard -z -c 0x000002d008000000 64 0 4 -d
saft-ecpu-ctl baseboard -z -c 0x0000033008000000 64 0 4 -d
saft-ecpu-ctl baseboard -z -c 0x0000037008000000 64 0 4 -d
sleep 1

# configure as ESR gateway
wr-mil-gw-ctl dev/wbm0 -s

# done
echo "wr-mil-gw started in ESR mode"
