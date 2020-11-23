#!/bin/sh
# startup script for timing receivers for ESR
#
set -x

###########################################
#dev/wbm0 -> tr0 -> trigger
###########################################
#export TRTRIG=$(saft-eb-fwd tr0)
export TRTRIG=dev/wbm0

###########################################
# clean up stuff
###########################################
#echo -e b2b-esr - start: bring possibly resident firmware to idle state
b2b-ctl $TRTRIG stopop
sleep 5

b2b-ctl $TRTRIG idle
sleep 5
echo -e b2b-esr - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x

echo -e b2b-esr - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w
saft-io-ctl tr0 -x 


###########################################
# load firmware to lm32
###########################################
echo -e b2b-esr - start: load firmware 
eb-fwload $TRTRIG u 0x0 b2bkd.bin

echo -e b2b-esr configure firmware
sleep 5
b2b-ctl $TRTRIG configure
sleep 5
b2b-ctl $TRTRIG startop


echo -e b2b-esr - start: configure for kicker diagnostic measurements
# IO2 configured as TLU input (from 'monitor')
# configure TLU !!! NO TERMINATION !!!
saft-io-ctl tr0 -n IO2 -o 0 -t 0
saft-io-ctl tr0 -n IO2 -b 0xffffa02000000000

# IO1 configured as TLU input (from 'probe')
# configure TLU
saft-io-ctl tr0 -n IO1 -o 0 -t 1
saft-io-ctl tr0 -n IO1 -b 0xffffa01000000000

# lm32 listens to TLU
# IO2 is monitor from electronics
# IO1 is probe from kicker magnet
# hack: to preserve order of the two signals on IO1, we must prevent
# the 1st signal being late by ADDING an offset of 20us
saft-ecpu-ctl tr0 -c 0xffffa01000000001 0xffffffffffffffff 20000 0xa01 -d
saft-ecpu-ctl tr0 -c 0xffffa02000000001 0xffffffffffffffff 0 0xa02 -d

echo -e b2b-esr - start: configure outputs
# configure outputs (ESR trigger)
# lm32 listens to CMD_B2B_TRIGGERINJ message from CBU
# as we need time to enable the input gates we SUBTRACT and offset of 20us
saft-ecpu-ctl tr0 -c  0x1154805000000000 0xfffffff000000000 20000 0x805 -d -g

# generate pulse upon CMD_B2B_TRIGGERINJ
saft-io-ctl tr0 -n IO3 -o 1 -t 0 -a 1
saft-io-ctl tr0 -n IO3 -c 0x1154805000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO3 -c 0x1154805000000000 0xfffffff000000000 1000 0x0 0 -u 

# generate test pulses upon CMD_B2B_TRIGGERINJ
saft-io-ctl tr0 -n IO5 -o 1 -t 0 -a 1
saft-io-ctl tr0 -n IO5 -c 0x1154805000000000 0xfffffff000000000 4000 0x0 1 -u
saft-io-ctl tr0 -n IO5 -c 0x1154805000000000 0xfffffff000000000 5000 0x0 0 -u 
saft-io-ctl tr0 -n IO6 -o 1 -t 0 -a 1
saft-io-ctl tr0 -n IO6 -c 0x1154805000000000 0xfffffff000000000 10000 0x0 1 -u
saft-io-ctl tr0 -n IO6 -c 0x1154805000000000 0xfffffff000000000 11000 0x0 0 -u 
saft-io-ctl tr0 -n IO6 -o 1 -t 0 -a 1
saft-io-ctl tr0 -n IO6 -c 0x1154805000000000 0xfffffff000000000 13000 0x0 1 -u
saft-io-ctl tr0 -n IO6 -c 0x1154805000000000 0xfffffff000000000 14000 0x0 0 -u 

