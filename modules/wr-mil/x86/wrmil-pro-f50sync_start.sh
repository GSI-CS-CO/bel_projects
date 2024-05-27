#!/bin/sh
# startup script for WRF50
#
#set -x

###########################################
# setting for production
# gateway : dev/wbm0, tr0
export TRGW=dev/wbm0         # EB device 
export SDGW=tr0              # saftlib device
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
wrf50-ctl $TRGW configure
sleep 2
wrf50-ctl $TRGW startop

###########################################
# configure ECA
###########################################
echo -e WRF50: configure $SDGW for timestamping, needed for monitoring
#  configured as TLU input (Lemo cable from MIL piggy)
saft-io-ctl $SDGW -n B1 -o 0 
saft-io-ctl $SDGW -n B1 -b 0x1fe1a01000000000

# lm32 listens to TLU
saft-ecpu-ctl $SDGW -c 0x1fe1a01000000001 0xffffffffffffffff 20000 0xa1 -d

# lm32 writes to MIL device via ECA wishbone channel
echo -e WRF50: configure $DGW wishbone channel, needed for writing telegrams to the MIL device
saft-wbm-ctl $SDGW -c 0x1ff0000000000000 0xffff000000000000 0 1 -d
saft-wbm-ctl $SDGW -r 1 $MILADDR 0 0x5f

# lm32 listens to timing messages for EVTNO 0x000..0x0ff
saft-ecpu-ctl $SDGW -c 0x1${SIDGW}000000000000 0xfffff00000000000 500000 0xff -g -d

###########################################
# reset diagnostics
###########################################
sleep 2
wrf50-ctl $TRGW cleardiag

