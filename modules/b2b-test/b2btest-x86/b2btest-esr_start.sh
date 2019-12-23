#!/bin/sh
# startup script for timing receiver ESR
#
# set -x

###########################################
# dev/wbm0 -> tr0 -> phase measurement

###########################################

###########################################
# clean up stuff
###########################################
echo -e b2btest-esr - start: bring possibly resident firmware to idle state
b2btest-ctl dev/wbm0 stopop
sleep 5

b2btest-ctl dev/wbm0 idle
sleep 5

echo -e b2btest-esr - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x

echo -e b2btest-esr - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w

###########################################
# load firmware to lm32
###########################################
echo -e b2btest-esr - start: load firmware
eb-fwload dev/wbm0 u 0x0 b2bpm.bin
sleep 5

echo -e b2btest-esr configure firmware
b2btest-ctl dev/wbm0 configure
sleep 5
b2btest-ctl dev/wbm0 startop
sleep 5

echo -e b2btest-esr - start: configure tr0 for phase measurement, TLU
################################################
# configure phase measurement input (SIS18 DDS)
################################################
# IO3 configured as TLU input (from 'DDS')
# configure TLU
saft-io-ctl tr0 -n IO3 -o 0 -t 1
saft-io-ctl tr0 -n IO3 -b 0xffff100000000000

# lm32 listens to TLU
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x2 -d

################################################
# configure lm32 and outputs (SIS18 -> ESR)
################################################
# lm32 listens to B2B_PMINJ message from CBU
saft-ecpu-ctl tr0 -c 0x1fa7802000000000 0xfffffff000000000 0 0x802 -d

# testing pulse upon B2B_DIAGINJ message from CBU
saft-io-ctl tr0 -n IO2 -o 1 -t 0
saft-io-ctl tr0 -n IO2 -c 0x1fa7806000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO2 -c 0x1fa7806000000000 0xfffffff000000000 10000000 0x0 0 -u

# testing pulse upon B2B_DIAGMATCH message from CBU
saft-io-ctl tr0 -n IO1 -o 1 -t 0
saft-io-ctl tr0 -n IO1 -c 0x1fa7807000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO1 -c 0x1fa7807000000000 0xfffffff000000000 10000000 0x0 0 -u



