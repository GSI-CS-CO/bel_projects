#!/bin/sh
# startup script for B2B
#
set -x

###########################################
# setting for production
# PS : dev/wbm0, tr0
# int: two Dual FIB 3 (= 4 DDS)
# pro: two single FIB3 (= 2 DDS)
export TRPS=dev/wbm0
export SDPS=tr0

# SCU bus slots
export SIS18DDS=0x44
export   ESRDDS=0x48

# registers
export  FWVERSION=000c            # FW version; useful when looking for modules on the scu bus
export  SHIFTPHAS=4000            # phase shift, value [degree, single precision float]
export  SHIFTTIME=4004            # phase shift, value [s, float]
export  SHIFTSTRT=4024            # phase shift, start
export   TAGREGLO=0612            # fg quad, tag config low bits
export   TAGREGHI=0614            # fg quad, tag config high bits
export TAGREGCTRL=0600            # fg quad, control register

# values
export     DDSENABLE=0xcc020000   # fg quad, value written to the control register for enabling phase reset
export DDSTAGRESETLO=4711         # tag value for phase reset, low word
export DDSTAGRESETHI=4711         # tag value for phase reset, high word


###########################################
# setting for development
###########################################

echo -e B2B start script for all rf room, use a single SCU Crate for phase shift, INT
###########################################
# clean up stuff
###########################################
echo -e b2b: bring possibly resident firmware to idle state
b2b-ctl $TRPS stopop
sleep 2

b2b-ctl $TRPS idle
sleep 2

echo -e b2b: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl $SDPS -x

echo -e b2b: destroy all unowned conditions and delete all macros for wishbone channel of ECA
saft-wbm-ctl $SDPS -x

echo -e b2b: destroy all anowned conditions for scu bus event channel of ECA
saft-scu-ctl $SDPS -x

###########################################
# load firmware to lm32
###########################################
echo -e b2b: load firmware 
eb-fwload $TRPS u 0x0 b2bpsm.bin

echo -e b2b: configure firmware
sleep 2
b2b-ctl $TRPS configure
sleep 2
b2b-ctl $TRPS startop
sleep 2

# disabled as this leads to ambiguation; we need a dedicated user space
# program on the host to count phase shifts
#echo -e b2b: configure $SDPS lm32 channel, needed for counting phase shifts
# lm32 listens to phase shift @SIS18 fast extraction
saft-ecpu-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 0 0x80a -d
# lm32 listens to phase shift @SIS18 transfer to ESR
saft-ecpu-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 0 0x80a -d
# lm32 listens to phase shift @SIS18 transfer to SIS100
saft-ecpu-ctl $SDPS -c 0x13a280a000000000 0xfffffff000000000 0 0x80a -d
# lm32 listens to phase shift @ESR injection transfer from SIS18
saft-ecpu-ctl $SDPS -c 0x13a180b000000000 0xfffffff000000000 0 0x80b -d
# lm32 listens to phase shift @ESR fast extraction
saft-ecpu-ctl $SDPS -c 0x13a580a000000000 0xfffffff000000000 0 0x80a -d
# lm32 listens to phase shift @ESR transfer to CRYRING
saft-ecpu-ctl $SDPS -c 0x13a680a000000000 0xfffffff000000000 0 0x80a -d
# lm32 listens to phase shift @SIS100 injection from SIS18
saft-ecpu-ctl $SDPS -c 0x13a280b000000000 0xfffffff000000000 0 0x80b -d
# lm32 listens to phase shift @SIS100 fast extraction
saft-ecpu-ctl $SDPS -c 0x13b080a000000000 0xfffffff000000000 0 0x80a -d

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
#
echo -e record macros
#       -------------
# SIS18
# phase shift, phase
saft-wbm-ctl $SDPS -r 0x00 $SIS18DDS$SHIFTPHAS 0 0x5c
saft-wbm-ctl $SDPS -r 0x01 $SIS18DDS$SHIFTPHAS 0 0x53
# phase shift, time
saft-wbm-ctl $SDPS -r 0x02 $SIS18DDS$SHIFTTIME 0 0x4c
saft-wbm-ctl $SDPS -r 0x03 $SIS18DDS$SHIFTTIME 0 0x43
# phase shift, start, EvtID 0x...........1....
saft-wbm-ctl $SDPS -r 0x04 $SIS18DDS$SHIFTSTRT 0 0x3c
#
# ESR
# phase shift, phase
saft-wbm-ctl $SDPS -r 0x05 $ESRDDS$SHIFTPHAS 0 0x5c
saft-wbm-ctl $SDPS -r 0x06 $ESRDDS$SHIFTPHAS 0 0x53
# phase shift, time
saft-wbm-ctl $SDPS -r 0x07 $ESRDDS$SHIFTTIME 0 0x4c
saft-wbm-ctl $SDPS -r 0x08 $ESRDDS$SHIFTTIME 0 0x43
# phase shift, start, EvtID 0x...........1....
saft-wbm-ctl $SDPS -r 0x09 $ESRDDS$SHIFTSTRT 0 0x3c
#
# SIS100
# ... (to be done)
#
echo -e ECA rules SIS18
#       ---------------
# mode B2B_MODE_B2EPSHIFT
# phase shift, phase
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 40000 0x00 -dg
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 30000 0x01 -dg 
# phase shift, time
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 20000 0x02 -dg
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000 10000 0x03 -dg
# phase shift, start
saft-wbm-ctl $SDPS -c 0x13a080a000000000 0xfffffff000000000     0 0x04 -d
# mode B2B_MODE_B2BPSHIFTE
# phase shift, phase
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 40000 0x00 -dg
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 30000 0x01 -dg 
# phase shift, time
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 20000 0x02 -dg
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000 10000 0x03 -dg
# phase shift, start
saft-wbm-ctl $SDPS -c 0x13a180a000000000 0xfffffff000000000     0 0x04 -d

echo -e ECA rules ESR
#       -------------
# mode B2B_MODE_B2EPSHIFT
# phase shift, phase
saft-wbm-ctl $SDPS -c 0x13a580a000000000 0xfffffff000000000 40000 0x05 -dg
saft-wbm-ctl $SDPS -c 0x13a580a000000000 0xfffffff000000000 30000 0x06 -dg
# phase shift, time
saft-wbm-ctl $SDPS -c 0x13a580a000000000 0xfffffff000000000 20000 0x07 -dg
saft-wbm-ctl $SDPS -c 0x13a580a000000000 0xfffffff000000000 10000 0x08 -dg
# phase shift, start
saft-wbm-ctl $SDPS -c 0x13a580a000000000 0xfffffff000000000     0 0x09 -d
# mode B2B_MODE_B2BPSHIFTI
# phase shift, phase
saft-wbm-ctl $SDPS -c 0x13a180b000000000 0xfffffff000000000 40000 0x05 -dg
saft-wbm-ctl $SDPS -c 0x13a180b000000000 0xfffffff000000000 30000 0x06 -dg 
# phase shift, time
saft-wbm-ctl $SDPS -c 0x13a180b000000000 0xfffffff000000000 20000 0x07 -dg
saft-wbm-ctl $SDPS -c 0x13a180b000000000 0xfffffff000000000 10000 0x08 -dg
# phase shift, start
saft-wbm-ctl $SDPS -c 0x13a180b000000000 0xfffffff000000000     0 0x09 -d

echo -e B2B: configure $SDPS scu bus tag channel, needed for DDS phase reset
echo -e SIS18, ESR, SIS100
# will be replaced by saft-wbm-ctl
# configure DDS to react on tag for phase reset
eb-write $TRPS $SIS18DDS$TAGREGLO/2 0x$DDSTAGRESETLO
eb-write $TRPS $SIS18DDS$TAGREGHI/2 0x$DDSTAGRESETHI
eb-write $TRPS $ESRDDS$TAGREGLO/2   0x$DDSTAGRESETLO
eb-write $TRPS $ESRDDS$TAGREGHI/2   0x$DDSTAGRESETHI
# SIS100 ... (to be done)
#
# SIS18
# configure ECA wishbone channel to enable phase reset at DDS, the 'enable' is done 10us prior CMD_PHASE_RESET
saft-wbm-ctl tr0 -c 0x112c159000000000  0xfffffff000000000 10000 0x0a -dg
saft-wbm-ctl tr0 -r 0x0a $SIS18DDS$TAGREGCTRL $DDSENABLE 0x0c
echo $DDSENABLE
# configure ECA tag channel to write tag upon receiving CMD_PHASE_RESET
saft-scu-ctl $SDPS -c 0x112c159000000000 0xfffffff000000000 0 0x$DDSTAGRESETHI$DDSTAGRESETLO -d
# ESR
# configure ECA wishbone channel to enable phase reset at DDS, the 'enable' is done 10us prior CMD_PHASE_RESET
saft-wbm-ctl tr0 -c 0x1154159000000000  0xfffffff000000000 10000 0x0b -dg
saft-wbm-ctl tr0 -r 0x0b $ESRDDS$TAGREGCTRL $DDSENABLE 0x0c
# configure ECA tag channel to write tag upon receiving CMD_PHASE_RESET
saft-scu-ctl $SDPS -c 0x1154159000000000 0xfffffff000000000 0 0x$DDSTAGRESETHI$DDSTAGRESETLO -d





