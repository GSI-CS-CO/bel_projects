#!/bin/bash
eb-fwload $1 u0 0 ../ftmfw/ftm.bin
./dm-sched $1 -w unipz.dot 
./dm-cmd $1 unipz.dot origin Evt_TKREQ_00
./dm-cmd $1 unipz.dot start

