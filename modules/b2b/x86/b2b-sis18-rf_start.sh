#!/bin/sh
# startup script for timing receivers for SIS18
#
set -x

###########################################
#dev/wbm0 -> tr0 -> phase measurement
#dev/wbm1 -> tr1 -> CBU
###########################################
export TRPM=$(saft-eb-fwd tr0)
export TRCBU=$(saft-eb-fwd tr1)
#export TRPM=dev/wbm0
#export TRCBU=dev/wbm1

###########################################
# clean up stuff
###########################################
echo -e b2b-sis18 - start: bring possibly resident firmware to idle state
b2b-ctl $TRPM stopop
b2b-ctl $TRCBU stopop
sleep 5

b2b-ctl $TRPM idle
b2b-ctl $TRCBU idle
sleep 5
echo -e b2b-sis18 - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x
saft-ecpu-ctl tr1 -x

echo -e b2b-sis18 - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w
saft-io-ctl tr1 -w
saft-io-ctl tr0 -x
saft-io-ctl tr1 -x


###########################################
# load firmware to lm32
###########################################
echo -e b2b-sis18 - start: load firmware 
eb-fwload $TRPM u 0x0 b2bpm.bin
eb-fwload $TRCBU u 0x0 b2bcbu.bin

echo -e b2b-sis18 configure firmware
sleep 5
b2b-ctl $TRPM configure
sleep 5
b2b-ctl $TRPM startop
sleep 5
b2b-ctl $TRCBU configure
sleep 5
b2b-ctl $TRCBU startop

echo -e b2b-sis18 - start: configure tr0 for phase measurement TLU
###########################
# configure PM Unit (SIS18)
###########################
# IO3 configured as TLU input (from 'DDS')
# configure TLU
saft-io-ctl tr0 -n IO3 -o 0 -t 1
saft-io-ctl tr0 -n IO3 -b 0xffff100000000000

# lm32 listens to TLU
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x2 -d

# lm32 listens to CMD_B2B_PMEXT  message from CBU
saft-ecpu-ctl tr0 -c 0x13a0800000000000 0xfffffff000000000 500000 0x800 -dg
saft-ecpu-ctl tr0 -c 0x13a1800000000000 0xfffffff000000000 500000 0x800 -dg

# testing pulse upon CMD_B2B_TRIGGEREXT message from CBU
saft-io-ctl tr0 -n IO2 -o 1 -t 1
saft-io-ctl tr0 -n IO2 -c 0x112c804000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO2 -c 0x112c804000000000 0xfffffff000000000 10000000 0x0 0 -u

# testing pulse upon CMD_B2B_DIAGEXT message from CBU
saft-io-ctl tr0 -n IO1 -o 1 -t 0
saft-io-ctl tr0 -n IO1 -c 0x13a0807000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO1 -c 0x13a0807000000000 0xfffffff000000000 10000000 0x0 0 -u
saft-io-ctl tr0 -n IO1 -c 0x13a1807000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO1 -c 0x13a1807000000000 0xfffffff000000000 10000000 0x0 0 -u
# add SIS100 later

echo -e b2b-sis18 - start: configure tr1 as cbu
##############################
# configure CBU (SIS18 -> ESR)
##############################
# IO1 generates TTL for EVT_KICK_START1 event
# convenience for triggering scope
saft-io-ctl tr1 -n IO1 -o 1 -t 0
saft-io-ctl tr1 -n IO1 -c 0x112c031000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr1 -n IO1 -c 0x112c031000000000 0xfffffff000000000 10000000 0x0 0 -u

# lm32 listens to EVT_KICK_START1  message from DM, 500us pretrigger
saft-ecpu-ctl tr1 -c 0x112c031000000000 0xfffffff000000000 500000 0x031 -dg

# SIS18_B2B_EXTRACT
# lm32 listens to B2B_PREXT message from extraction machine
saft-ecpu-ctl tr1 -c 0x13a0802000000000 0xfffffff000000000 500000 0x802 -dg

# SIS18_B2B_ESR
# lm32 listens to B2B_PREXT message from extraction machine
saft-ecpu-ctl tr1 -c 0x13a1802000000000 0xfffffff000000000 0 0x802 -d
# lm32 listens to B2B_PRINJ message from injection machine
saft-ecpu-ctl tr1 -c 0x13a1803000000000 0xfffffff000000000 0 0x803 -d

