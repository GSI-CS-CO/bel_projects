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
#b2b-ctl $TRPM stopop
#b2b-ctl $TRCBU stopop
#sleep 5

#b2b-ctl $TRPM idle
#b2b-ctl $TRCBU idle
#sleep 5
#echo -e b2b-esr - start: destroy all unowned conditions for lm32 channel of ECA
#saft-ecpu-ctl tr0 -x

echo -e b2b-esr - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w
#saft-io-ctl tr1 -w

echo -e b2b-esr - start: destroy all unowned conditions for IOs
saft-io-ctl tr0 -x

###########################################
# load firmware to lm32
###########################################
#echo -e b2b-esr - start: load firmware 
#eb-fwload $TRPM u 0x0 b2bpm.bin
#eb-fwload $TRCBU u 0x0 b2bcbu.bin

#echo -e b2b-esr configure firmware
#sleep 5
#b2b-ctl $TRPM configure
#sleep 5
#b2b-ctl $TRPM startop
#sleep 5
#b2b-ctl $TRCBU configure
#sleep 5
#b2b-ctl $TRCBU startop

echo -e b2b-esr - start: configure outputs
################################################
# configure outputs (ESR trigger)
################################################
# lm32 listens to B2B_PMEXT message from CBU
#saft-ecpu-ctl tr0 -c 0x1fa7801000000000 0xfffffff000000000 0 0x801 -d

# generate pulse upon CMD_B2B_TRIGGERINJ 
saft-io-ctl tr0 -n IO3 -o 1 -t 0 -a 1
saft-io-ctl tr0 -n IO2 -o 0 -t 0 -j 1
saft-io-ctl tr0 -n IO3 -c 0x1154805000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO3 -c 0x1154805000000000 0xfffffff000000000 1000 0x0 0 -u 
