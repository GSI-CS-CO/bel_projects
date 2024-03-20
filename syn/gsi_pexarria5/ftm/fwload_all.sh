#!/bin/sh
# starts and configures the firmware (lm32) of DM

###########################################
# load firmware to lm32
###########################################
echo -e DM - start: load firmware
if [ "$#" -lt 2 ]; then
  echo "Usage: $0 <device> <bin file> [<number CPUs>]" >&2
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

if [ "$#" -eq 3 ]; then
  CPU=$3
else
  CPU=$(eb-ls $DEV | grep 'LM32-RAM-User' -c)
fi

echo -e DM - start $DEV $FILE for $CPU CPUs

eb-reset $DEV cpuhalt 0xff
sleep 0.5
count=0
while [ $count -lt $CPU ]; do
  eb-fwload $DEV u$count 0 $FILE
  count=$(( $count + 1 ))
done
sleep 0.5
eb-reset $DEV cpureset 0xff
sleep 5

echo -e DM - start: clear hw diagnostics
dm-cmd $DEV cleardiag
echo -e DM - start: startup script finished


