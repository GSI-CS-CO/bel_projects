#!/bin/sh
# startup script for B2B
#
#set -x

###########################################
# setting for production
# KD : dev/wbm0, tr0
export TRTRIG=dev/wbm0
export SDTRIG=tr0
export SDTRIGTEST=tr1
###########################################
# setting for development
# ! don't forget to (un)comment test pulses
# at the end of this file
# KD : dev/ttyUSB1, tr0
#export TRTRIG=$(saft-eb-fwd tr0)
#export SDTRIG=tr0
###########################################

echo -e B2B start script for ESR kicker room

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


echo -e b2b: configure $SDTRIG as KD
###########################################
# configure KD
###########################################
echo -e b2b: configure for kicker diagnostic measurements
# IO2 configured as TLU input (from 'monitor') !!! NO TERMINATION !!!
saft-io-ctl $SDTRIG -n IO2 -o 0 -t 0
saft-io-ctl $SDTRIG -n IO2 -b 0xffffa02000000000

# IO1 configured as TLU input (from 'probe', extraction)
saft-io-ctl $SDTRIG -n IO1 -o 0 -t 1
saft-io-ctl $SDTRIG -n IO1 -b 0xffffa01000000000

# IO4 configured as TLU input (from 'probe', injection)
saft-io-ctl $SDTRIG -n IO4 -o 0 -t 1
saft-io-ctl $SDTRIG -n IO4 -b 0xffffa04000000000

# lm32 listens to TLU
# to preserve order of the signals on IO1/IO4 (probe) compared
# to IO2 (monitor), an offset is added for IO1/IO4
saft-ecpu-ctl $SDTRIG -c 0xffffa02000000001 0xffffffffffffffff 0 0xa02 -d
saft-ecpu-ctl $SDTRIG -c 0xffffa01000000001 0xffffffffffffffff 20000 0xa01 -d
saft-ecpu-ctl $SDTRIG -c 0xffffa04000000001 0xffffffffffffffff 20000 0xa04 -d

# INJECTION: lm32 listens to CMD_B2B_TRIGGERINJ message from CBU
# need pre-trigger to open input gates for probe signal
saft-ecpu-ctl $SDTRIG -c  0x1154805000000000 0xfffffff000000000 20000 0x805 -d -g
# EXTRACTION: lm32 listens to CMD_B2B_TRIGGEREXT message from CBU
# need pre-trigger to open input gates for probe signal
saft-ecpu-ctl $SDTRIG -c  0x1154804000000000 0xfffffff000000000 20000 0x804 -d -g

echo -e b2b: configure outputs
saft-io-ctl $SDTRIG -n IO3 -o 1 -t 0 -a 1
# INJECTION: generate pulse upon CMD_B2B_TRIGGERINJ
saft-io-ctl $SDTRIG -n IO3 -c 0x1154805000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl $SDTRIG -n IO3 -c 0x1154805000000000 0xfffffff000000000 1000 0x0 0 -u
# EXTRACTION: generate pulse upon CMD_B2B_TRIGGEREXT
saft-io-ctl $SDTRIG -n IO3 -c 0x1154804000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl $SDTRIG -n IO3 -c 0x1154804000000000 0xfffffff000000000 1000 0x0 0 -u

# generate test pulses upon CMD_B2B_TRIGGERINJ (IO2,IO1) and CMD_B2B_TRIGGEREXT (IO2, N/A)
saft-io-ctl $SDTRIGTEST -n IO2 -o 1 -t 0 -a 1
saft-io-ctl $SDTRIGTEST -n IO2 -c 0x1154805000000000 0xfffffff000000000 3200 0x0 1 -u
saft-io-ctl $SDTRIGTEST -n IO2 -c 0x1154805000000000 0xfffffff000000000 3400 0x0 0 -u
saft-io-ctl $SDTRIGTEST -n IO1 -o 1 -t 0 -a 1
saft-io-ctl $SDTRIGTEST -n IO1 -c 0x1154805000000000 0xfffffff000000000 4200 0x0 1 -u
saft-io-ctl $SDTRIGTEST -n IO1 -c 0x1154805000000000 0xfffffff000000000 4400 0x0 0 -u
saft-io-ctl $SDTRIGTEST -n IO2 -o 1 -t 0 -a 1
saft-io-ctl $SDTRIGTEST -n IO2 -c 0x1154804000000000 0xfffffff000000000 6400 0x0 1 -u
saft-io-ctl $SDTRIGTEST -n IO2 -c 0x1154804000000000 0xfffffff000000000 6600 0x0 0 -u
# config test pulse for probe extraction
# config test pulse for probe extraction
# config test pulse for probe extraction
