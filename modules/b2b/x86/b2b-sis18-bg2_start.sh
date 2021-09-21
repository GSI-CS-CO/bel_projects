#!/bin/sh
# startup script for B2B
#
set -x

###########################################
# setting for production
# CBU: dev/wbm0, tr0
export TRCBU=dev/wbm0
export SDCBU=tr0
###########################################
# setting for development
# CBU: dev/ttyUSB1, tr1
#export TRCBU=$(saft-eb-fwd tr1)
#export SDCBU=tr1
###########################################

echo -e B2B start script for SIS18 located at bg2 room

###########################################
# clean up stuff
###########################################
echo -e b2b: bring possibly resident firmware to idle state
b2b-ctl $TRCBU stopop
sleep 2

b2b-ctl $TRCBU idle
sleep 2

echo -e b2b: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDCBU -x

echo -e b2b: disable all events from I/O inputs to ECA
saft-io-ctl $SDCBU -w
saft-io-ctl $SDCBU -x

###########################################
# load firmware to lm32
###########################################
echo -e b2b: load firmware 
eb-fwload $TRCBU u 0x0 b2bcbu.bin

echo -e b2b: configure firmware
sleep 2
b2b-ctl $TRCBU configure
sleep 2
b2b-ctl $TRCBU startop

echo -e b2b: configure $SDCBU as cbu
###########################################
# configure CBU
###########################################
# lm32 listens to EVT_KICK_START1  message from DM, 500us pretrigger
saft-ecpu-ctl $SDCBU -c 0x112c031000000000 0xfffffff000000000 500000 0x031 -dg

# lm32 listens to CMD_B2B_PREXT message from extraction machine
saft-ecpu-ctl $SDCBU -c 0x13a0802000000000 0xfffffff000000000 500000 0x802 -dg
saft-ecpu-ctl $SDCBU -c 0x13a1802000000000 0xfffffff000000000 500000 0x802 -dg

# lm32 listens to CMD_B2B_PRINJ message from injection machine, only required for B2B -> later
saft-ecpu-ctl $SDCBU -c 0x13a1803000000000 0xfffffff000000000 500000 0x803 -dg

# diag: generate pulse upon EVT_KICK_START event
saft-io-ctl $SDCBU -n IO1 -o 1 -t 0
saft-io-ctl $SDCBU -n IO1 -c 0x112c031000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl $SDCBU -n IO1 -c 0x112c031000000000 0xfffffff000000000 10000000 0x0 0 -u

