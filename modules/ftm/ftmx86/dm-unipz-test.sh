#!/bin/bash
eb-fwload $1 u0 0 /home/mkreider/projects/bel_projects/modules/ftm/ftmfw/ftm.bin 
./test $1 unipz.dot -w
#eb-write $1 0x4110538/4 0x10001310; eb-write tcp/tsl008.acc.gsi.de 0x4110528/4 0x1

