DEV=$1

eb-reset $DEV cpuhalt 0xff
sleep 0.5
eb-fwload $DEV u0 0 ftm.bin
eb-fwload $DEV u1 0 ftm.bin
eb-fwload $DEV u2 0 ftm.bin
eb-fwload $DEV u3 0 ftm.bin
sleep 0.5
eb-reset $DEV cpureset 0xff

