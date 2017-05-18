#!/bin/bash

# find wishbone address of second LM32 user ram:
BASE_ADR=`eb-ls tcp/scuxl0089.acc.gsi.de | grep LM32-RAM-User | tail -n 1 | sed 's/  */ /g' | cut -d" " -f3`
CMD_ADR=`expr $BASE_ADR + 500 + 8`  # 500 is relative start of shared segment, 8 is the address of command word

# write a 1 into the command register to stop the lm32
eb-write tcp/scuxl0089.acc.gsi.de 0x${CMD_ADR}/4 0x1 
sleep 1
eb-fwload tcp/scuxl0089.acc.gsi.de u1 0 wr_mil.bin 
