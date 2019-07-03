#!/bin/bash

DIR=$2
DEV=$1


RSTBASE="0x`eb-ls $DEV | grep -Po "(?:3a362063\s*)\K(\d+)"`"
RSTSET=`printf "0x%X\n" $((RSTBASE + 0x4))`
RSTCLR=`printf "0x%X\n" $((RSTBASE + 0x8))`
eb-write $DEV $RST/4 0xf

#Get adresses for user lm32 rams (Dev ID 54111351)
RAMS=`eb-ls $DEV | grep -Po "(?:54111351\s*)\K(\d+)"`
#memory dump for all rams as 'memdump_<i>.bin'
i=0
for LINE in $RAMS; do
 #TODO: calculate RAM size from difference of first and last address
 eb-put $DEV 0x$LINE $DIR/memdump_$i.bin
 let i+=1
done  

#
#EOF
