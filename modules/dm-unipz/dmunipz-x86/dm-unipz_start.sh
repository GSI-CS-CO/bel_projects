#!/bin/sh
# starts and configures the firmware (lm32) and software (host) of dm-unipz 

# set -x

###########################################
# clean up stuff
###########################################
echo -e dm-unipz (start): bring possibly resident firmware to idle state
dmunipz-ctl dev/wbm0 stopop
sleep 5

dmunipz-ctl dev/wbm0 idle
sleep 5
echo -e dm-unipz (start): destroy all unowned conditions for lm32 channel of ECA
saft-ecpu-ctl baseboard -x


###########################################
# load firmware to lm32
###########################################
echo -e dm-unipz (start): load firmware
eb-fwload dev/wbm0 u 0x0 dmunipz.bin

###########################################
# start software on hostsystem 
###########################################
# to be implemented: command with pipe to logger (logstash)

###########################################
# configure firmware and make it operational 
###########################################
 
# do some write actions to set register values
# to be implemented

echo -e dm-unipz (start): make firmware operational
# send CONFIGURE command to firmware
sleep 5
dmunipz-ctl dev/wbm0 configure

# send START OPERATATION command to firmware
sleep 5
dmunipz-ctl dev/wbm0 startop

###########################################
# configure ECA
###########################################
echo -e dm-unipz (start): configure lm32 channel of ECA

# configure ECA for lm32 channel: here action for TK request, tag "0x2"
saft-ecpu-ctl baseboard -c 0x2222000000000000 0xffff000000000000 0 0x2 -d

# configure ECA for lm32 channel: here action for TK request, tag "0x3"
saft-ecpu-ctl baseboard -c 0x3333000000000000 0xffff000000000000 0 0x3 -d

# configure ECA for lm32 channel: here action for TK request, tag "0x4"
saft-ecpu-ctl baseboard -c 0x4444000000000000 0xffff000000000000 0 0x4 -d

echo -e dm-unipz (start): startup script finished

###########################################
# testing without datamaster
###########################################
# saft-dm bla -fp -n 3600000 schedule.txt
# alternative : 
# - saft-ctl bla -fp inject 0x2222000000000002 0x0 0
# - saft-ctl bla -fp inject 0x3333000000000002 0x0 0
# note that action of firmware is triggered by tag and
# that virtual accelerator is specified by low bits of EvtID

