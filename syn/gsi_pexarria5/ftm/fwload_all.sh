DEV=$1
FILE=$2

eb-reset $DEV cpuhalt 0xff
sleep 0.5
eb-fwload $DEV u0 0 $FILE
eb-fwload $DEV u1 0 $FILE
eb-fwload $DEV u2 0 $FILE
eb-fwload $DEV u3 0 $FILE
sleep 0.5
eb-reset $DEV cpureset 0xff

