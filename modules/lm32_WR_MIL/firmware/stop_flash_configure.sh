#!/bin/bash

SHARED_SEGMENT_OFFSET=0x500 #  0x500 is relative start of shared segment
COMMAND_OFFSET=0x4          #  0x4   is the address of command word counting from start of shared segment
SOURCE_OFFSET=0x14          #  0x14  is the address of the event source register (has to be written 1 (SIS) or 2 (ESR))
MAGIC_OFFSET=0x0   

# find wishbone address of second LM32 user ram:
BASE_ADR=`eb-ls tcp/scuxl0089.acc.gsi.de | grep LM32-RAM-User | tail -n 1 | sed 's/  */ /g' | cut -d" " -f3`
CMD_ADR=`printf "%x\n" $[0x$BASE_ADR + $SHARED_SEGMENT_OFFSET + $COMMAND_OFFSET]` 
SOURCE_ADR=`printf "%x\n" $[0x$BASE_ADR + $SHARED_SEGMENT_OFFSET + $SOURCE_OFFSET]` 
MAGIC_ADR=`printf "%x\n" $[0x$BASE_ADR + $SHARED_SEGMENT_OFFSET + $MAGIC_OFFSET]` 

# write a 1 into the command register to stop the lm32
eb-write tcp/scuxl0089.acc.gsi.de 0x${CMD_ADR}/4 0x1 

sleep 1
# program new firmware
eb-fwload tcp/scuxl0089.acc.gsi.de u1 0 wr_mil.bin 

sleep 1
# configure the wr-mil-gw to be SIS
#eb-write tcp/scuxl0089.acc.gsi.de 0x${CMD_ADR}/4 0x3 

sleep 1 
# read magic number
eb-read tcp/scuxl0089.acc.gsi.de 0x${MAGIC_ADR}/4
