#!/bin/sh
# starts and configures the firmware (lm32) and software (host) of dm-unipz 

# set -x

###########################################
# clean up stuff
###########################################
echo -e dm-unipz - start: bring possibly resident firmware to idle state
dmunipz-ctl dev/wbm0 stopop
sleep 5

dmunipz-ctl dev/wbm0 idle
sleep 5
echo -e dm-unipz - start: destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl tr0 -x
echo -e dm-unipz - start: disable all events from I/O inputs to ECA
saft-io-ctl tr0 -w

###########################################
# load firmware to lm32
###########################################
echo -e dm-unipz - start: load firmware
eb-fwload dev/wbm0 u 0x0 dmunipz.bin

###########################################
# start software on hostsystem 
###########################################
echo dm-unipz - start: kill monitoring process
killall dmunipz-ctl
echo dm-unipz - start: start  monitoring
dmunipz-ctl -s1 dev/wbm0 | logger -t dmunipz-ctl -sp local0.info &

###########################################
# configure firmware and make it operational 
###########################################

# convention: test system (tsl404 as DM) uses:
# - 192.168.11.2 ( c0a80b02 ) has ip for SCU, MAC check with eb-mon 
# - 192.168.11.1 ( c0a80b01 ) has ip for DM,  MAC tsl404: 0x00267b000455
#
# some data masters
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000408 0xc0a88040 (tsl015, user network) 
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b00046b 0xc0a880f7 (tsl017, production network)
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000422 0xc0a80c04 (tsl008, 'Hanno network')
# dmunipz-ctl dev/wbm0 ebmdm 0x00267b000455 0xc0a80b01 (tsl404, 'Testnetz Dietrich')
#
# some SCUs
# dmunipz-ctl dev/wbm0 ebmlocal 0x00267b000321 0xc0a8a00c (scuxl0033, user network)
# dmunipz-ctl dev/wbm0 ebmlocal 0x00267b0003f1 0xc0a8a0e5 (scuxl0223, production network)
# dmunipz-ctl dev/wbm0 ebmlocal 0x00267b000321 0xc0a80cea (scuxl0033, 'Hanno network')
# dmunipz-ctl dev/wbm0 ebmlocal 0x00267b000321 0xc0a80b02 (scuxl0033, 'Testnetz Dietrich', need to set static IP with eb-console)
# dmunipz-ctl dev/wbm0 ebmlocal 0x00267b0003f1 0xc0a80b02 (scuxl0223, 'Testnetz Dietrich', need to set static IP with eb-console)

# do some write actions to set register values
echo -e dm-unipz - start: set MAC and IP of gateway and Data Master

PROSCU=scuxl0223

if  [ $(hostname) == $PROSCU ]; then   # production network
    echo -e dm-unipz - start: configuring for PRODUCTION network on $(hostname)
    dmunipz-ctl dev/wbm0 ebmdm 0x00267b00046b 0xc0a880f7
    dmunipz-ctl dev/wbm0 ebmlocal 0x00267b0003f1 0xc0a8a0e5
else                                  # test or development
    echo -e dm-unipz - start: configuring for TEST or DEVELOPMENT network on $(hostname)
    dmunipz-ctl dev/wbm0 ebmlocal 0x00267b000321 0xc0a80cea
    dmunipz-ctl dev/wbm0 ebmdm 0x00267b000422 0xc0a80c04
fi

echo -e dm-unipz - start: make firmware operational

# send CONFIGURE command to firmware
sleep 5
dmunipz-ctl dev/wbm0 configure

###########################################
# configure ECA for DM
###########################################
echo -e dm-unipz - start: configure ECA for events from DM

# configure ECA for lm32 channel: listen for TK request, tag "0x2"
saft-ecpu-ctl tr0 -c 0x112c15e000000000 0xfffffff000000000 0 0x2 -d

# configure ECA for lm32 channel: listen for beam request, tag "0x3"
saft-ecpu-ctl tr0 -c 0x112c160000000000 0xfffffff000000000 0 0x3 -d

# configure ECA for lm32 channel: listen for TK release, tag "0x4"
saft-ecpu-ctl tr0 -c 0x112c15f000000000 0xfffffff000000000 0 0x4 -d

# configure ECA for lm32 channel: listen for MB Load (TK 7 Chopper), tag "0x7"
saft-ecpu-ctl tr0 -c 0x112c028000000000 0xfffffff000000000 0 0x7 -d


###########################################
# configure TLU and ECA for UNIPZ
# MIL event EVT_READY_TO_SIS is received as TTL
###########################################
echo -e dm-unipz - start: configure TLU and ECA for I/O events from UNIPZ

# configure TLU (input B1, TLU will generate messages with event ID
saft-io-ctl tr0 -n B1 -b 0xffff100000000000

# configure ECA for lm32 channel: listen for event ID from TLU, tag "0x6"
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x6 -d

# send START OPERATATION command to firmware
sleep 5
echo -e dm-unipz - start: start operation
dmunipz-ctl dev/wbm0 startop

echo -e dm-unipz - start: startup script finished

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

