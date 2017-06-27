#!/bin/bash

SHARED_SEGMENT_OFFSET=0x500 # 500 is relative start of shared segment
COMMAND_OFFSET=0x0          #   0 is the address of command word counting from start of shared segment

# find wishbone address of second LM32 user ram:
BASE_ADR=`eb-ls tcp/scuxl0089.acc.gsi.de | grep LM32-RAM-User | tail -n 1 | sed 's/  */ /g' | cut -d" " -f3`
CMD_ADR=`printf "%x\n" $[0x$BASE_ADR + $SHARED_SEGMENT_OFFSET + $COMMAND_OFFSET]` 

# write a 1 into the command register to stop the lm32
eb-write tcp/scuxl0089.acc.gsi.de 0x${CMD_ADR}/4 0x1 
#eb-write tcp/scuxl0089.acc.gsi.de 0x200a0508/4 0x1
sleep 1
eb-fwload tcp/scuxl0089.acc.gsi.de u1 0 wr_mil.bin 
