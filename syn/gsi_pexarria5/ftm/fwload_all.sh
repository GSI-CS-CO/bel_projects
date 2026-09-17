#!/bin/sh
# starts and configures the firmware (lm32) of DM

###########################################
# load firmware to lm32
###########################################
PREFIX="DM - start:"
echo $PREFIX load firmware
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
  if $(command -v eb-ls > /dev/null); then
    CPU=$(eb-ls $DEV | grep 'LM32-RAM-User' -c)
    # echo $PREFIX Found $CPU CPUs with eb-ls at $(command -v eb-ls)
  else
    CPU=4
    # echo $PREFIX Set $CPU CPUs, eb-ls not found
  fi
fi

echo $PREFIX load $FILE to device $DEV for $CPU CPUs

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

echo $PREFIX clear hw diagnostics
dm-cmd $DEV cleardiag
echo $PREFIX load firmware finished
