#!/bin/sh
# startup script for B2B
#
#set -x

###########################################
# setting for production
# PM : dev/wbm0, tr0
# CBU: dev/wbm1, tr1
export TRPM=dev/wbm0
export SDPM=tr0
export TRCBU=dev/wbm1
export SDCBU=tr1
###########################################
# setting for development
# PM : dev/ttyUSB1, tr0
# CBU: dev/wbm0, tr1
#export TRPM=$(saft-eb-fwd tr0)
#export SDPM=tr0
#export TRCBU=$(saft-eb-fwd tr1)
#export SDCBU=tr1
###########################################

echo -e B2B start script for ESR rf room INT

###########################################
# clean up stuff
###########################################
echo -e b2b: bring possibly resident firmware to idle state
b2b-ctl $TRPM stopop
b2b-ctl $TRCBU stopop
sleep 2

b2b-ctl $TRPM idle
b2b-ctl $TRCBU idle
sleep 2

echo -e b2b: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDPM -x
saft-ecpu-ctl $SDCBU -x

echo -e b2b: disable all events from I/O inputs to ECA
saft-io-ctl $SDPM -w
saft-io-ctl $SDCBU -w
saft-io-ctl $SDPM -x
saft-io-ctl $SDCBU -x

###########################################
# load firmware to lm32
###########################################
echo -e b2b: load firmware 
eb-fwload $TRPM u 0x0 b2bpm.bin
eb-fwload $TRCBU u 0x0 b2bcbu.bin

echo -e b2b: configure firmware
sleep 2
b2b-ctl $TRPM configure
sleep 2
b2b-ctl $TRPM startop
sleep 2
b2b-ctl $TRCBU configure
sleep 2
b2b-ctl $TRCBU startop

echo -e b2b: configure $SDPM for phase measurement TLU
###########################################
# configure PM
###########################################
# IO3 configured as TLU input (from 'DDS')
saft-io-ctl $SDPM -n IO3 -o 0 -t 1
saft-io-ctl $SDPM -n IO3 -b 0xffffa03000000000

# lm32 listens to TLU
saft-ecpu-ctl $SDPM -c 0xffffa03000000001 0xffffffffffffffff 0 0xa03 -d

# lm32 listens to CMD_B2B_PMINJ message from SIS18 CBU
saft-ecpu-ctl $SDPM -c 0x13a1801000000000 0xfffffff000000000 0 0x801 -d

# lm32 listens to CMD_B2B_PMEXT message from ESR CBU 
saft-ecpu-ctl $SDPM -c 0x13a5800000000000 0xfffffff000000000 0 0x800 -d
saft-ecpu-ctl $SDPM -c 0x13a6800000000000 0xfffffff000000000 0 0x800 -d

# lm32 listens to CMD_B2B_TRIGGERINJ message from SIS18 CBU - match diagnostic
saft-ecpu-ctl $SDPM -c 0x1154805000000000 0xfffffff000000000 20000 0x805 -dg

# lm32 listens to CMD_B2B_TRIGGEREXT message from ESR CBU - match diagnostic
saft-ecpu-ctl $SDPM -c 0x1154804000000000 0xfffffff000000000 20000 0x804 -dg

# lm32 listens to >>delayed<< (CMD_B2B_PMINJ) message from SIS18 CBU: B2B_ECADO_B2B_PDINJ - phase diagnostic
saft-ecpu-ctl $SDPM -c 0x13a1801000000000 0xfffffff000000000 15900000 0x821 -d

# lm32 listens to >>delayed<< (CMD_B2B_PMEXT) message from ESR CBU: B2B_ECADO_B2B_PDEXT - phase diagnostic
saft-ecpu-ctl $SDPM -c 0x13a5800000000000 0xfffffff000000000 15900000 0x820 -d
saft-ecpu-ctl $SDPM -c 0x13a6800000000000 0xfffffff000000000 15900000 0x820 -d

# diag: generate pulse upon CMD_B2B_TRIGGERINJ message from SIS18 CBU
# sacrifice IO1 for 'jitter check'
#saft-io-ctl $SDPM -n IO1 -o 1 -t 0
#saft-io-ctl $SDPM -n IO1 -c 0x1154805000000000 0xfffffff000000000 0 0x0 1 -u
#saft-io-ctl $SDPM -n IO1 -c 0x1154805000000000 0xfffffff000000000 10000000 0x0 0 -u

# diag: generate pulse upon CMD_B2B_TRIGGEREXT message from ESR CBU
saft-io-ctl $SDPM -n IO2 -o 1 -t 0
saft-io-ctl $SDPM -n IO2 -c 0x1154804000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl $SDPM -n IO2 -c 0x1154804000000000 0xfffffff000000000 10000000 0x0 0 -u

echo -e b2b: configure $SDCBU as cbu
###########################################
# configure CBU
###########################################
# lm32 listens to CMD_B2B_START  message from DM
saft-ecpu-ctl $SDCBU -c 0x115481f000000000 0xfffffff000000000 0 0x81f -d

# lm32 listens to CMD_B2B_PREXT message from extraction machine, 250us pretrigger
saft-ecpu-ctl $SDCBU -c 0x13a5802000000000 0xfffffff000000000 250000 0x802 -dg
saft-ecpu-ctl $SDCBU -c 0x13a6802000000000 0xfffffff000000000 250000 0x802 -dg

# lm32 listens to CMD_B2B_PRINJ message from injection machine, only for B2B
saft-ecpu-ctl $SDCBU -c 0x13a6803000000000 0xfffffff000000000 0 0x803 -d

# diag: generate pulse upon CMD_B2B_START event
# sacrifice IO1 for 'jitter check'
#saft-io-ctl $SDCBU -n IO1 -o 1 -t 0
#saft-io-ctl $SDCBU -n IO1 -c 0x115481f000000000 0xfffffff000000000 0 0x0 1 -u
#saft-io-ctl $SDCBU -n IO1 -c 0x115481f000000000 0xfffffff000000000 10000000 0x0 0 -u
