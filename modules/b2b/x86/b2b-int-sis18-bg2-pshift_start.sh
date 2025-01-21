#!/bin/sh
# startup script for B2B
#
set -x

###########################################
# setting for production
# PS : dev/wbm0, tr0
export TRPS=dev/wbm0
export SDPS=tr0
export DDSSHIFTPHAS=0x444000    # phase shift, value
export DDSSHIFTTIME=0x444004    # phase shift, value [s, float]
export DDSSHIFTSTRT=0x444024    # phase shift, start
export DDSTAGREGLO=0x440612     # fg quad, tag config low bits
export DDSTAGREGHI=0x440614     # fg quad, tag config high bits
export DDSTAGREGCTRL=0x440600   # fg quad, control register
export DDSTAGCTRL=0xcc020000    # fg quad, value written to the control register for enabling phase reset
export DDSTAGRESET=0x47114711   # tag value for phase reset
export DDSTAGRESETLO=0x4711     # tag value for phase reset, low word
export DDSTAGRESETHI=0x4711     # tag value for phase reset, high word


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

echo -e b2b: destroy all anowned conditions for scu bus event channel of ECA
saft-scu-ctl $SDPS -x

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
# -- 0x3: copies LOW 16bit of selected 32bits to HIGH 16 bits of the selected address on the SCU bus
# -- 0xc: copies HIGH 16bit of selected 32bits to LOW 16 bits of the selected address on the SCU bus
# example for phase shift value
# - DDS is in slot #2
# - registers for  phase shift value
# -- 0x4000: low bits  (16 bit address 0x2000)
# -- 0x4002: high bits (16 bit address 0x2001)
# - 0x444000: wishbone address of phase shift low bits (this address is 32 bit aligned)
# timing messages with parameter field 0xaaaabbbbccccdddd
# when using flags 0x53
#                    ||--- ssss: copies low 16 bits of selected data
#                    |-----dddd: selectes low 32 bits of parameter field
# the 16 bit value 0xdddd is copied to the wishbone address 0x444002 !
# when using flags 0x5c
#                    ||--- ssss: copies high 16 bits of selected data
#                    |-----dddd: selectes low 32 bits of parameter field
# the 16 bit value 0xcccc is copied to the wishbone address 0x444000 !
# ToDo: Swap endianess when copying the data to the parameter field
echo -e B2B: configure $SDPS wishbone channel, needed for writing duration and value for phase shift
# parameter field 0xaaaabbbbccccdddd
# shift value low bits: 0xcccc
# mode B2B_MODE_B2EPSHIFT
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 40000 1 -dg
saft-wbm-ctl $SDPS -r 1 $DDSSHIFTPHAS 0 0x5c
# shift value high bits: 0xdddd
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 30000 2 -dg 
saft-wbm-ctl $SDPS -r 2 $DDSSHIFTPHAS 0 0x53
# shift time low bits: 0xbbbb
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 20000 3 -dg
saft-wbm-ctl $SDPS -r 3 $DDSSHIFTTIME 0 0x4c
# shift time high bits: 0xaaaa
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 10000 4 -dg
saft-wbm-ctl $SDPS -r 4 $DDSSHIFTTIME 0 0x43
# start phase shift, EvtID 0x...........1....
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000     0 5 -d
saft-wbm-ctl $SDPS -r 5 $DDSSHIFTSTRT - 0x3c
# mode B2B_MODE_B2BPSHIFTE
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 40000 6 -dg
saft-wbm-ctl $SDPS -r 6 $DDSSHIFTPHAS 0 0x5c
# shift value high bits: 0xdddd
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 30000 7 -dg 
saft-wbm-ctl $SDPS -r 7 $DDSSHIFTPHAS 0 0x53
# shift time low bits: 0xbbbb
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 20000 8 -dg
saft-wbm-ctl $SDPS -r 8 $DDSSHIFTTIME 0 0x4c
# shift time high bits: 0xaaaa
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 10000 9 -dg
saft-wbm-ctl $SDPS -r 9 $DDSSHIFTTIME 0 0x43
# start phase shift, EvtID 0x...........1....
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000     0 10 -d
saft-wbm-ctl $SDPS -r 10 $DDSSHIFTSTRT - 0x3c

echo -e B2B: configure $SDPS scu bus tag channel, needed for DDS phase reset
# configure DDS to react on tag for phase reset
eb-write $TRPS $DDSTAGREGLO/2 $DDSTAGRESETLO
eb-write $TRPS $DDSTAGREGHI/2 $DDSTAGRESETHI
# configure ECA wishbone channel to enable phase reset at DDS, the 'enable' is done 10us prior CMD_PHASE_RESET
saft-wbm-ctl tr0 -c 0x112c159000000000  0xfffffff000000000 10000 11 -dg
saft-wbm-ctl tr0 -r 11 $DDSTAGREGCTRL $DDSTAGCTRL 0x0c
# configure ECA tag channel to write tag upon receiving CMD_PHASE_RESET
saft-scu-ctl $SDPS -c 0x112c159000000000 0xfffffff000000000 0 $DDSTAGRESET -d



