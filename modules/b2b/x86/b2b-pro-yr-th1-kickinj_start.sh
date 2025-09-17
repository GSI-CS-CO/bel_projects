#!/bin/sh
# startup script for B2B
#
set -x

###########################################
# setting for production
# KD : dev/wbm0, tr0
export TRTRIG=dev/wbm0
export SDTRIG=tr0
export SDKICK=tr0
###########################################
# setting for development
# ! don't forget to (un)comment test pulses
# at the end of this file
# KD : dev/ttyUSB0, tr2
#export TRTRIG=$(saft-eb-fwd tr2)
#export SDTRIG=tr2
#export SDKICK=tr2
###########################################

echo -e B2B start script for CRYRING cave kicker injection

###########################################
# clean up stuff
###########################################
echo -e b2b: bring possibly resident firmware to idle state
b2b-ctl $TRTRIG stopop
sleep 2

b2b-ctl $TRTRIG idle

sleep 2
echo -e b2b: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDTRIG -x

echo -e b2b: disable all events from I/O inputs to ECA
saft-io-ctl $SDTRIG -w
saft-io-ctl $SDTRIG -x 


###########################################
# load firmware to lm32
###########################################
echo -e b2b: load firmware 
eb-fwload $TRTRIG u 0x0 b2bkd.bin

echo -e b2b: configure firmware
sleep 2
b2b-ctl $TRTRIG configure
sleep 2
b2b-ctl $TRTRIG startop


echo -e b2b: configure tr0 as KD
###########################################
# configure KD
###########################################
echo -e b2b: configure for kicker diagnostic measurements
# IO2 configured as TLU input (from 'monitor') !!! NO TERMINATION !!!
saft-io-ctl $SDTRIG -n IO2 -o 0 -t 0
saft-io-ctl $SDTRIG -n IO2 -b 0xffffa02000000000

# IO1 configured as TLU input (from 'probe')
saft-io-ctl $SDTRIG -n IO1 -o 0 -t 1
saft-io-ctl $SDTRIG -n IO1 -b 0xffffa01000000000

# lm32 listens to TLU
# to preserve order of signals on IO1, (probe) compared
# to IO2 (monitor), an offset is added for IO1
saft-ecpu-ctl $SDTRIG -c 0xffffa02000000001 0xffffffffffffffff 0 0xa02 -d
saft-ecpu-ctl $SDTRIG -c 0xffffa01000000001 0xffffffffffffffff 20000 0xa01 -d

# lm32 listens to CMD_B2B_TRIGGERINJ message from CBU
# need pre-trigger to open input gates for probe signal
saft-ecpu-ctl $SDTRIG -c  0x10d2805000000000 0xfffffff000000000 20000 0x805 -d -g

echo -e b2b: configure outputs
# generate pulse upon CMD_B2B_TRIGGERINJ
saft-io-ctl $SDKICK -n IO3 -o 1 -t 0 -a 1
saft-io-ctl $SDKICK -n IO3 -c 0x10d2805000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl $SDKICK -n IO3 -c 0x10d2805000000000 0xfffffff000000000 1000 0x0 0 -u 

# generate test pulses upon CMD_B2B_TRIGGERINJ (sorry: requires PEXP, do this later)
