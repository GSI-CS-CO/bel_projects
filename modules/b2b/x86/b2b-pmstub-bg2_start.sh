#!/bin/sh
# startup script for B2B
#
set -x

###########################################
# setting for production
# PM : dev/wbm0, tr0
# CBU: dev/wbm1, tr1
export TRPM=dev/wbm0
export SDPM=tr0
###########################################
# setting for development
# PM : dev/ttyUSB0, tr0
# CBU: dev/ttyUSB1, tr1
#export TRPM=$(saft-eb-fwd tr0)
#export SDPM=tr0
###########################################

echo -e B2B start script for PM stub at BG2 room

###########################################
# clean up stuff
###########################################
echo -e b2b: bring possibly resident firmware to idle state
b2b-ctl $TRPM stopop
sleep 2

b2b-ctl $TRPM idle
sleep 2

echo -e b2b: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDPM -x

echo -e b2b: disable all events from I/O inputs to ECA
saft-io-ctl $SDPM -w
saft-io-ctl $SDPM -x

###########################################
# load firmware to lm32
###########################################
echo -e b2b: load firmware 
eb-fwload $TRPM u 0x0 b2bpmstub.bin

echo -e b2b: configure firmware
sleep 2
b2b-ctl $TRPM configure
sleep 2
b2b-ctl $TRPM startop
sleep 2

echo -e b2b: configure $SDPM for phase measurement TLU
###########################################
# configure PM
###########################################
# lm32 listens to TLU
saft-ecpu-ctl $SDPM -c 0xffffa03000000001 0xffffffffffffffff 0 0xa03 -d

# lm32 listens to CMD_B2B_PMEXT message from SIS18 CBU
saft-ecpu-ctl $SDPM -c 0x13a0800000000000 0xfffffff000000000 0 0x800 -d
saft-ecpu-ctl $SDPM -c 0x13a1800000000000 0xfffffff000000000 0 0x800 -d

# lm32 listens to CMD_B2B_TRIGGEREXT message from SIS18 CBU - match diagnostic
saft-ecpu-ctl $SDPM -c 0x112c804000000000 0xfffffff000000000 20000 0x804 -dg

# lm32 listens to >> 9.9ms delayed<< (CMD_B2B_PMEXT) message from SIS18 CBU: B2B_ECADO_B2B_PDEXT - phase diagnostic
saft-ecpu-ctl $SDPM -c 0x13a0800000000000 0xfffffff000000000 9900000 0x820 -d
saft-ecpu-ctl $SDPM -c 0x13a1800000000000 0xfffffff000000000 9900000 0x820  -d

# diag: generate pulse upon CMD_B2B_TRIGGEREXT message from SIS18 CBU
saft-io-ctl $SDPM -n IO1 -o 1 -t 0
saft-io-ctl $SDPM -n IO1 -c 0x112c804000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl $SDPM -n IO1 -c 0x112c804000000000 0xfffffff000000000 10000000 0x0 0 -u

