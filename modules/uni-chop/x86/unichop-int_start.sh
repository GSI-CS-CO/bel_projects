#!/bin/sh
# startup script for UNICHOP
#
#set -x

###########################################
# setting for production
# gateway : dev/wbm0, tr0
export TRGW=dev/wbm0         # EB device 
export SDGW=tr0              # saftlib device
export MILDEV=1              # MIL device, piggy(0), sio slot 1 (1) ...
# piggy: Wishbone address of 'GSI_MIL_SCU'
#        the register 0x1004 needs to be added
#        exammple piggy: 0x9004 @ fallout Gateware
# SIO  : Wishbone address of 'SCU-BUS-Master' + (slot number) * 0x20000
#        the register 0x800 needs to be added
#        example SIO slot #1: 0x420800
###########################################
# setting for development
# gateway: N/A
###########################################

echo -e UNICHOP start script for GID 0x$SIDGW

###########################################
# clean up stuff
###########################################
echo -e UNICHOP: bring possibly resident firmware to idle state
unichop-ctl $TRGW stopop
sleep 2

unichop-ctl $TRGW idle
sleep 2

echo -e UNICHOP: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDGW -x

echo -e UNICHOP: disable all events from I/O inputs to ECA
saft-io-ctl $SDGW -w

echo -e UNICHOP: destroy all unowned conditions and delete all macros for wishbone channel of ECA
saft-wbm-ctl $SDGW -x

###########################################
# load firmware to lm32 and configure fw
###########################################
echo -e UNICHOP: load firmware 
eb-fwload $TRGW u 0x0 unichop.bin

echo -e UNICHOP: configure firmware for gateway $NGW
sleep 2
unichop-ctl $TRGW -w$MILDEV configure
sleep 2
unichop-ctl $TRGW startop

###########################################
# configure ECA
###########################################

# lm32 listens to timing messages for EVTNO 0x000..0x0ff
saft-ecpu-ctl $SDGW -c 0x1ff0fa0000000000 0xffffffff00000000 1000000 0xfa0 -g -d

###########################################
# reset diagnostics
###########################################
sleep 1
unichop-ctl $TRGW cleardiag
