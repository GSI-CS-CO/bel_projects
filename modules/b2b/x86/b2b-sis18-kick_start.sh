#!/bin/sh
# startup script for timing receivers for SIS18
#
set -x

###########################################
#dev/wbm0 -> tr0 -> trigger
###########################################
#export TRTRIG=$(saft-eb-fwd tr0)
export TRTRIG=dev/ttyUSB0

###########################################
# clean up stuff
###########################################
#echo -e b2b-sis18 - start: bring possibly resident firmware to idle state
b2b-ctl $TRTRIG stopop
sleep 5

b2b-ctl $TRTRIG idle
sleep 5
echo -e b2b-sis18 - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x

echo -e b2b-sis18 - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w -x 


###########################################
# load firmware to lm32
###########################################
echo -e b2b-sis18 - start: load firmware 
eb-fwload $TRTRIG u 0x0 b2bkd.bin

echo -e b2b-sis18 configure firmware
sleep 5
b2b-ctl $TRTRIG configure
sleep 5
b2b-ctl $TRTRIG startop


echo -e b2b-sis18 - start: configure for kicker diagnostic measurements
# IO2 configured as TLU input (from 'monitor')
# configure TLU !!! NO TERMINATION !!!
saft-io-ctl tr0 -n IO2 -o 0 -t 0
saft-io-ctl tr0 -n IO2 -b 0xffff100000000000

# IO1 configured as TLU input (from 'probe')
# configure TLU
saft-io-ctl tr0 -n IO1 -o 0 -t 1
saft-io-ctl tr0 -n IO1 -b 0xffff100000000000

# lm32 listens to TLU
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x2 -d

echo -e b2b-sis18 - start: configure outputs
# configure outputs (SIS18 trigger)
# lm32 listens to CMD_B2B_TRIGGEREXT message from CBU
saft-ecpu-ctl tr0 -c  0x112c804000000000 0xfffffff000000000 20000 0x804 -d -g

# generate pulse upon CMD_B2B_TRIGGEREXT
saft-io-ctl tr0 -n IO3 -o 1 -t 0 -a 1
saft-io-ctl tr0 -n IO3 -c 0x112c804000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO3 -c 0x112c804000000000 0xfffffff000000000 1000 0x0 0 -u 
