#!/bin/sh
# starts and configures the firmware (lm32) and software (host) of dm-unidm 

# set -x

###########################################
# clean up stuff
###########################################
echo -e dm-unidm - start: bring possibly resident firmware to idle state
dmunipz-ctl dev/wbm0 stopop
sleep 5

dmunipz-ctl dev/wbm0 idle
sleep 5
echo -e dm-unidm - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x
echo -e dm-unidm - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w

###########################################
# load firmware to lm32
###########################################
echo -e dm-unidm - start: load firmware
eb-fwload dev/wbm0 u 0x0 dmunidm.bin

###########################################
# start software on hostsystem 
###########################################
#echo dm-unidm - start: kill monitoring process
#killall dmunipz-ctl
#
#echo dm-unidm - start: start  monitoring
#/bin/daemon -NiU --name=dmunipz-daemon --pidfile=/var/run/dmunipz-ctl.pid --stdout=local0.info --stderr=local0.err -- dmunipz-ctl -s1 dev/wbm0

###########################################
# configure firmware and make it operational 
###########################################

# convention: test system (tsl404 as DM) uses:
# - 192.168.11.2 ( c0a80b02 ) has ip for SCU, MAC check with eb-mon 
# - 192.168.11.1 ( c0a80b01 ) has ip for DM,  MAC tsl404: 0x00267b000455
#
# some data masters
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000446 0xc0a880bc (tsl015 , user network) 
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000484 0xc0a88111 (tsl020 , integration network) 
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000422 0xc0a80c04 (tsl008 , integration network backup)
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000899 0xc0a8846d (fel0090, integration network another DM)
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b00046b 0xc0a880f7 (tsl017 , production network)
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b00045e 0xc0a880d4 (tsl018 , production network backup)
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000455 0xc0a80b01 (tsl404 , 'Testnetz Dietrich')
#

# do some write actions to set register values
echo -e dm-unidm - start: set MAC and IP of gateway and Data Master

dmunipz-ctl dev/wbm0 ebmdm 0x00267b000899 0xc0a8846d
dmunipz-ctl dev/wbm0 uni 10000

echo -e dm-unidm - start: make firmware operational

# send CONFIGURE command to firmware
sleep 5
dmunipz-ctl dev/wbm0 configure

###########################################
# configure ECA for DM
###########################################
echo -e dm-unidm - start: configure ECA for events from DM

# configure ECA for lm32 channel: listen for CMD_UNI_TCREQ, tag "0x2"
saft-ecpu-ctl tr0 -c 0x112c15e000000000 0xfffffff000000000 0 0x2 -d

# configure ECA for lm32 channel: listen for CMD_UNI_BREQ, tag "0x3"
saft-ecpu-ctl tr0 -c 0x112c160000000000 0xfffffff000000000 0 0x3 -d

# configure ECA for lm32 channel: listen for CMD_UNI_TCREL, tag "0x4"
saft-ecpu-ctl tr0 -c 0x112c15f000000000 0xfffffff000000000 0 0x4 -d

# configure ECA for lm32 channel: listen for EVT_MB_TRIGGER (SIS18 bumper magnets), tag "0x7"
saft-ecpu-ctl tr0 -c 0x112c028000000000 0xfffffff000000000 0 0x7 -d

# configure ECA for lm32 channel: listen for CMD_UNI_BPREP, tag "0x8"
saft-ecpu-ctl tr0 -c 0x112c161000000000 0xfffffff000000000 0 0x8 -d

# configure ECA for lm32 channel: listen for CMD_UNI_BREQ_NOWAIT, tag "0x9"
saft-ecpu-ctl tr0 -c 0x112c162000000000 0xfffffff000000000 0 0x9 -d

# configure ECA for lm32 channel: listen for EVT_READY_TO_SIS, tag "0xa" (for testing only)
saft-ecpu-ctl tr0 -c 0x11c601e000000000 0xfffffff000000000 0 0xa -d


###########################################
# configure TLU and ECA for UNIPZ
# MIL event EVT_READY_TO_SIS is received as TTL
###########################################
echo -e dm-unidm - start: configure TLU and ECA for I/O events from UNIPZ

# configure TLU (input B1, TLU will generate messages with event ID
saft-io-ctl tr0 -n B1 -b 0xffff100000000000

# configure ECA for lm32 channel: listen for event ID from TLU, tag "0x6"
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x6 -d

# send START OPERATATION command to firmware
sleep 5
echo -e dm-unidm - start: start operation
dmunipz-ctl dev/wbm0 startop

echo -e dm-unidm - start: startup script finished

sleep 2

dmunipz-ctl dev/wbm0 cleardiag


###########################################
# testing without datamaster
###########################################
# saft-dm bla -fp -n 3600000 schedule.txt
# alternative : 
# - saft-ctl bla -fp inject 0x2222000000000002 0x0 0
# - saft-ctl bla -fp inject 0x3333000000000002 0x0 500000000
# - saft-ctl bla -fp inject 0x4444000000000002 0x0 800000000
# note that action of firmware is triggered by tag and
# that virtual accelerator is specified by low bits of EvtID

