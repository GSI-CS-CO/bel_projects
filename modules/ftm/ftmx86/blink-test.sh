#!/bin/bash
file=blink.dot

eb-fwload $1 u0 0 ../ftmfw/ftm.bin
sleep 1
./dm-sched $1 $file -w
sleep 1
./dm-cmd $1 $file origin -t0 Evt_POLICE0
./dm-cmd $1 $file origin -t1 Evt_FIREF0
./dm-cmd $1 $file preptime -t0 1000000000
./dm-cmd $1 $file preptime -t1 1000000000

./dm-cmd $1 $file starttime -t0 0000000
./dm-cmd $1 $file starttime -t1 0000000
./dm-cmd $1 $file start 0x3
sleep 1
./dm-cmd $1 $file heap



