#!/bin/sh
# startup script for B2B
#
set -x

###########################################
# setting for production
# PS : dev/wbm0, tr0
export TRPS=dev/wbm0
export SDPS=tr0
export DDSSHIFTPHAS=0x444000    # value of phase shift 
export DDSSHIFTTIME=0x444004    # duration of phase shift
export DDSSHIFTSTRT=0x444024    # register for start

###########################################
# setting for development
# to be done ...
###########################################

echo -e B2B start script for SIS18 rf room, phase shift,  INT
###########################################
# clean up stuff
###########################################
echo -e b2b: destroy all unowned conditions and delete all macros for wishbone channel of ECA
saft-wbm-ctl $SDPS -x

###########################################
# load firmware to lm32
###########################################
# future projet :-/

###########################################
# configure ECA
###########################################
# timing message writes to MIL device via ECA wishbone channel
# I (db) have difficulties understanding how it works with the select bits
# and the transfer from 32bits (of saft-wbm-ctl) to 16bits (SCU bus)
# presently (dec 2024) this only works change of endianess
#
# according to the command line help of saft-wbm-ctl <flags> is cddddssss
# - c I do not select (= 0)
# - dddd: 0x4 (high 32 bits) or 0x5 (low 32 bits) of parameter field
# a 'write' to a 32bit aligned address on the 16 bit of the SCU bus
# can be implemented as follows
# - ssss:
# -- 0x1: copies LOW 16bit of selected 32bits to HIGH 16 bits of the selected address on the SCU bus
# -- 0x2: copies HIGH 16bit of selected 32bits to LOW 16 bits of the selected address on the SCU bus
# example for phase shift value
# - DDS is in slot #2
# - registers for  phase shift value
# -- 0x4000: low bits  (16 bit address 0x2000)
# -- 0x4002: high bits (16 bit address 0x2001)
# - 0x444000: wishbone address of phase shift low bits (this address is 32 bit aligned)
# timing messages with parameter field 0xaaaabbbbccccdddd
# when using flags 0x51
#                    ||--- ssss: copies low 16 bits of selected data
#                    |-----dddd: selectes low 32 bits of parameter field
# the 16 bit value 0xdddd is copied to the wishbone address 0x444002 !
# when using flags 0x52
#                    ||--- ssss: copies high 16 bits of selected data
#                    |-----dddd: selectes low 32 bits of parameter field
# the 16 bit value 0xcccc is copied to the wishbone address 0x444000 !
# ToDo: Swap endianess when copying the data to the parameter field
echo -e B2B: configure $SDPS wishbone channel, needed for writing duration and value for phase shift
# parameter field 0xaaaabbbbccccdddd
# shift value low bits: 0xcccc
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 40000 1 -dg
saft-wbm-ctl $SDPS -r 1 $DDSSHIFTPHAS 0 0x52
# shift value high bits: 0xdddd
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 30000 2 -dg 
saft-wbm-ctl $SDPS -r 2 $DDSSHIFTPHAS 0 0x51
# shift time low bits: 0xbbbb
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 20000 3 -dg
saft-wbm-ctl $SDPS -r 3 $DDSSHIFTTIME 0 0x42
# shift time high bits: 0xaaaa
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 10000 4 -dg
saft-wbm-ctl $SDPS -r 4 $DDSSHIFTTIME 0 0x41
# start phase shift, EvtID 0x...........1....
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000     0 5 -d
saft-wbm-ctl $SDPS -r 5 $DDSSHIFTSTRT - 0x32




