#!/bin/sh
# starts and configures the firmware (lm32) of DM

###########################################
# load firmware to lm32
###########################################
echo -e DM - start: load firmware
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <device> <bin file>" >&2
  exit 1
fi
if ! [ -e "$2" ]; then
  echo "bin file $2 not found" >&2
  exit 1
fi
if ! [ -f "$2" ]; then
  echo "bin file $2 is not a file" >&2
  exit 1
fi

DEV=$1
FILE=$2

echo -e DM - start $DEV $FILE

eb-reset $DEV cpuhalt 0xff
sleep 0.5
eb-fwload $DEV u0 0 $FILE
eb-fwload $DEV u1 0 $FILE
eb-fwload $DEV u2 0 $FILE
eb-fwload $DEV u3 0 $FILE
sleep 0.5
eb-reset $DEV cpureset 0xff
sleep 5

echo -e DM - start: clear hw diagnostics
dm-cmd $DEV cleardiag
echo -e DM - start: startup script finished


