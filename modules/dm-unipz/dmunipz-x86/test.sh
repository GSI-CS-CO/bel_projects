#!/bin/sh
dmunipz-ctl dev/wbm0 stopop
sleep 1
dmunipz-ctl dev/wbm0 idle
sleep 1
eb-fwload dev/wbm0 u 0x0 dmunipz.bin
dmunipz-ctl dev/wbm0 ebmdm 0x00267b000422 0xc0a80c04
dmunipz-ctl dev/wbm0 ebmlocal 0x00267b000321 0xc0a80cea
dmunipz-ctl dev/wbm0 configure
sleep 1
dmunipz-ctl dev/wbm0 startop
