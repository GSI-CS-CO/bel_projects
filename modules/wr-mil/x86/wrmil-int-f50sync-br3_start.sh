#!/bin/sh
# startup script for WRF50
#
#set -x

###########################################
# setting for production, here: UNI-INT
# gateway : dev/wbm0, tr0
export TRGW=dev/wbm1         # EB device 
export SDGW=tr1              # saftlib device
###########################################
# setting for development
# gateway: N/A
###########################################

echo -e WRF50 start script

###########################################
# clean up stuff
###########################################
echo -e WRF50: bring possibly resident firmware to idle state
wrf50-ctl $TRGW stopop
sleep 2

wrf50-ctl $TRGW idle
sleep 2

echo -e WRF50: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDGW -x

echo -e WRF50: disable all events from I/O inputs to ECA
saft-io-ctl $SDGW -w

echo -e WRF50: destroy all unowned conditions and delete all macros for wishbone channel of ECA
saft-wbm-ctl $SDGW -x

###########################################
# load firmware to lm32 and configure fw
###########################################
echo -e WRF50: load firmware 
eb-fwload $TRGW u 0x0 wrf50.bin

echo -e WRF50: configure firmware for gateway $NGW
sleep 2
wrf50-ctl -m4 $TRGW configure
sleep 2
wrf50-ctl $TRGW startop

###########################################
# configure ECA
###########################################
echo -e WRF50: configure $SDGW for timestamping, needed for monitoring
#  configured as TLU input (Lemo cable from MIL piggy)
saft-io-ctl $SDGW -n IO1 -o 0 -t 1
saft-io-ctl $SDGW -n IO1 -b 0x14c0a01000000000

# lm32 listens to TLU
saft-ecpu-ctl $SDGW -c 0x14c0a01000000001 0xffffffffffffffff 500000 0xa01 -d

# lm32 listens to timing messages from Data Master
saft-ecpu-ctl $SDGW -c 0x14c0fc0000000000 0xfffffff000000000 0 0xfc0  -d

###########################################
# reset diagnostics
###########################################
sleep 5
wrf50-ctl $TRGW cleardiag

