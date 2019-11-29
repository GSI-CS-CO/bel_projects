#!/bin/sh
# startup script for timing receivers for SIS18
#
# set -x

###########################################
# dev/wbm0 -> tr0 -> phase measurement
# dev/wbm1 -> tr1 -> CBU
###########################################

###########################################
# clean up stuff
###########################################
echo -e b2btest-sis18 - start: bring possibly resident firmware to idle state
b2btest-ctl dev/wbm0 stopop
b2btest-ctl dev/wbm1 stopop
sleep 5

b2btest-ctl dev/wbm0 idle
b2btest-ctl dev/wbm1 idle
sleep 5
echo -e b2btest-sis18 - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x
saft-ecpu-ctl tr1 -x

echo -e b2btest-sis18 - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w
saft-io-ctl tr1 -w

###########################################
# load firmware to lm32
###########################################
echo -e b2btest-sis18 - start: load firmware
eb-fwload dev/wbm0 u 0x0 b2bpm.bin
eb-fwload dev/wbm1 u 0x0 b2bcbu.bin

echo -e b2btest-sis18 - start: configure tr0 for phase measurement (TlU)
################################################
# configure phase measurement input (SIS18 DDS)
################################################
# IO3 configured as TLU input (from 'DDS')
# configure TLU 
saft-io-ctl tr0 -n IO3 -b 0xffff100000000000
#eb-write dev/wbm0 0x4012000/4 0x0004

# lm32 listens to TLU
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x2 -d

################################################
# configure lm32 and outputs (SIS18 -> ESR)
################################################
# lm32 listens to B2B_PMEXT message from CBU
saft-ecpu-ctl tr0 -c 0x1fa7801000000000 0xfffffff000000000 0 0x801 -d

# testing pulse upon B2B_DIAGEXT message from CBU
saft-io-ctl tr0 -n IO2 -o 1 -t 0
saft-io-ctl tr0 -n IO2 -c 0x1fa7805000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO2 -c 0x1fa7805000000000 0xfffffff000000000 10000000 0x0 0 -u

# testing pulse upon B2B_DIAGMATCH message from CBU
saft-io-ctl tr0 -n IO1 -o 1 -t 0
saft-io-ctl tr0 -n IO1 -c 0x1fa7807000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO1 -c 0x1fa7807000000000 0xfffffff000000000 10000000 0x0 0 -u

echo -e b2btest-sis18 - start: configure tr1 as cbu
################################################
# configure CBU (SIS18 -> ESR)
################################################
# IO1 generates TTL for B2B_START event
# convenience for triggering scope
saft-io-ctl tr1 -n IO1 -o 1 -t 0
saft-io-ctl tr1 -n IO1 -c 0x1fa7800000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr1 -n IO1 -c 0x1fa7800000000000 0xfffffff000000000 10000000 0x0 0 -u

# lm32 listens to B2B_START message from DM
saft-ecpu-ctl tr1 -c 0x1fa7800000000000 0xfffffff000000000 0 0x800 -d

# lm32 listens to B2B_PREXT message from extraction machine
saft-ecpu-ctl tr1 -c 0x1fa7803000000000 0xfffffff000000000 0 0x803 -d

# lm32 listens to B2B_PRINJ message from injection machine
saft-ecpu-ctl tr1 -c 0x1fa7804000000000 0xfffffff000000000 0 0x804 -d



