#!/bin/sh
# startup script for UNIBLM
#
#set -x

###########################################
# setting for integration
# gateway : dev/wbm0, tr0
export TRGW=dev/wbm0         # EB device 
export SDGW=tr0              # saftlib device
###########################################
# setting for development
# gateway: N/A
###########################################

echo -e UNIBLM start script for UNILAC BLM

###########################################
# clean up stuff
###########################################
echo -e UNIBLM: bring possibly resident firmware to idle state
uniblm-ctl $TRGW stopop
sleep 2

uniblm-ctl $TRGW idle
sleep 2

echo -e UNIBLM: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDGW -x

#echo -e UNIBLM: disable all events from I/O inputs to ECA
#saft-io-ctl $SDGW -w

#echo -e UNIBLM: destroy all unowned conditions and delete all macros for wishbone channel of ECA
#saft-wbm-ctl $SDGW -x

###########################################
# load firmware to lm32 and configure fw
###########################################
echo -e UNIBLM: load firmware 
eb-fwload $TRGW u 0x0 uniblm.bin

echo -e UNIBLM: configure firmware for gateway $NGW
sleep 2
uniblm-ctl $TRGW configure
sleep 2
uniblm-ctl $TRGW startop

###########################################
# configuration of the ECA is done dynamically
# by the userspace application
###########################################

###########################################
# reset diagnostics
###########################################
sleep 2
uniblm-ctl $TRGW cleardiag
