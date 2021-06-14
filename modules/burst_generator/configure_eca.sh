#!/bin/sh
# debug enable
#set -x

#echo $#
#echo $@

get_usr_answer()
{
  read usr_answer
  if [ "$usr_answer" != "y" ] && [ "$usr_answer" != "Y" ]; then
    echo Sorry, user disagreed. Exit!
    exit 1
  fi
}

write_pulse_params_to_ram()
{
  echo writing event id, delay, conditions, offset to RAM:

  local USR_RAM_ADDR=$1
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 ${IO_EVNT_ID::10}
  printf ' io evnt id   @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $PULSE_DELAY
  printf ' delay        @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $IO_RULES
  printf ' conditions   @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $IO_BLOCK_PERIOD
  printf ' block period @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  # instruct LM32 to load params from RAM
  eb-write dev/wbm0 ${2}/4 2

  echo
}

write_prod_cycles_to_ram()
{
  echo writing production cycles to RAM:

  local USR_RAM_ADDR=$1
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 ${IO_EVNT_ID::10}
  printf ' io evnt id   @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $PULSE_CYCLE_HI32
  printf ' cycle_hi32   @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $PULSE_CYCLE_LO32
  printf ' cycle_lo32   @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  # instruct LM32 to load production cycles from RAM
  eb-write dev/wbm0 ${2}/4 3

  echo
}

############## globals #########################################################

TR_NAME=""
IO_NAME="IO1"

# default number of ECA rules for a pulse block
IO_RULES=10

# default length of the signal active state, nanoseconds
IO_ACT_LEN=1000

# default period of a signal, nanoseconds (IO_ACT_LEN < IO_PERIOD)
IO_PERIOD=2000


# default event flags (delayed)
IO_EVNT_FLG=0x8

IO_EVNT_ID=0x0000FCA000000000
IO_EVNT_MSK=0xFFFFFFF000000000

PULSE_DELAY=0;
PULSE_CYCLE_HI32=0
PULSE_CYCLE_LO32=5 # inject timing messages for IO action 5 times

OPT_SHARED_RAM=0
OPT_USR_INTERACTION=1

if [ $# -ne 0 ]; then
  while getopts ":hn:a:b:f:c:d:p:u" opt; do
    case $opt in
      h ) # help
        echo "$0 <options>"
        echo  "options:"
        echo
        echo "-a <active>       active state length of a signal, nanoseconds"
        echo "-b <period>       signal period, nanoseconds"
        echo "-n <conditions>   number of conditions (pulse block)"
        echo "-f <flags>        condition flags (1:?, 2:?, 4: ?, 8: ?)"
        echo "-c <cycles, h32>  pulse cycles (high 32-bit value), nanoseconds"
        echo "-d <cycles, l32>  pulse cycles (low 32-bit value)"
        echo "-p <mb_slot addr> send parameters to LM32 using shared RAM"
        echo "-u                user interaction is not needed"
        exit 0 ;;
      n ) # number of ECA conditions (builds up a pulse block)
        IO_RULES=$OPTARG ;;
      a ) # active state length of a signal
        IO_ACT_LEN=$OPTARG ;;
      b ) # period of a signal
        IO_PERIOD=$OPTARG
        if [ $IO_PERIOD -le $IO_ACT_LEN ]; then
          echo "Invalid signal period (less or equal to active state of $IO_ACT_LEN)"
          exit 1
        fi ;;
      f ) # flags
        IO_EVNT_FLG=$OPTARG ;;
      c ) # pulse cycle high 32-bit
        PULSE_CYCLE_HI32=$OPTARG ;;
      d ) # pulse cycle low 32-bit
        PULSE_CYCLE_LO32=$OPTARG ;;
      p ) # allow to store the pulse params to RAM
        OPT_SHARED_RAM=1
        OPT_SHARED_RAM_ARGS=$OPTARG ;;
      u ) # disable user interaction
        OPT_USR_INTERACTION=0 ;;
      : )
        echo "Bad option: $OPTARG requires an argument" 1>&2
        exit 1 ;;
      \? )
        echo "Invalid option: $OPTARG" 1>&2
        exit 1 ;;
    esac
  done
  shift $((OPTIND -1))
fi

remainder=$( expr $IO_RULES % 2 )
periods=$( expr $IO_RULES / 2 )

# pulse block period, nanoseconds
IO_BLOCK_PERIOD=$( expr $periods \* $IO_PERIOD )

if [ $remainder -eq 1 ]; then
  IO_BLOCK_PERIOD=$( expr $IO_BLOCK_PERIOD + $IO_ACT_LEN )
fi

echo "pulse block period = $IO_BLOCK_PERIOD ns"

############# write pulse parameters to shared memory #########################

# write conditions and offset to shared RAM

if [ $OPT_SHARED_RAM -eq 1 ]; then
  write_pulse_params_to_ram 0x200a0510 $OPT_SHARED_RAM_ARGS
  write_prod_cycles_to_ram 0x200a0510 $OPT_SHARED_RAM_ARGS
fi
############# set action rules for LM32 ########################################

LM32_EVNT_BG_ON=0x0000991000000000
LM32_EVNT_BG_OFF=0x0000990000000000
LM32_EVNT_MSK=0xFFFFFFF000000000
LM32_EVNT_TAG=0xb2b2b2b2
LM32_EVNT_OFF=0

if [ $OPT_USR_INTERACTION -eq 1 ]; then
  echo "Conditions below are required to generate pulses at chosen output:"
  echo " e_id: $LM32_EVNT_BG_ON, e_mask: $LM32_EVNT_MSK, offset: $LM32_EVNT_OFF, tag: $LM32_EVNT_TAG"
  echo " e_id: $LM32_EVNT_BG_OFF, e_mask: $LM32_EVNT_MSK, offset: $LM32_EVNT_OFF, tag: $LM32_EVNT_TAG"
  echo
  echo "Are you sure to set them in ECA for the LM32 action channel (y/n ?)"
  get_usr_answer
fi

#get the name of a timing receiver
TR_NAME=$(saft-ctl bla -f -j | grep -oE "name:(.*?)path" | sed s/,//g | cut -d" " -f2)

# clean current rules
saft-ecpu-ctl $TR_NAME -x

# set ECA rules for the LM32 action channel
saft-ecpu-ctl $TR_NAME -d -c $LM32_EVNT_BG_ON $LM32_EVNT_MSK $LM32_EVNT_OFF $LM32_EVNT_TAG
saft-ecpu-ctl $TR_NAME -d -c $LM32_EVNT_BG_OFF $LM32_EVNT_MSK $LM32_EVNT_OFF $LM32_EVNT_TAG

# verify ECA rules for the LM32 action channel
echo Current ECA rules the LM32 action channel
saft-ecpu-ctl $TR_NAME -l

echo

############## configure IO ####################################################

echo "Detecting available IO in system ..."
# list all IO and their capabilities
saft-io-ctl $TR_NAME -i

# choose the first available IO
AVAIL_IO=$(saft-io-ctl $TR_NAME -i | grep '\bOut\b' | head -1 | cut -d" " -f1)
if [ "$AVAIL_IO" != "" ]; then
  IO_NAME=$AVAIL_IO
fi

if [ $OPT_USR_INTERACTION -eq 1 ]; then
  # ask user agreement prior to configuration
  echo
  echo "Do you want to set $IO_NAME of $TR_NAME as output (y/n) ?"
  get_usr_answer

  echo "Conditions below are required to generate pulses at output $IO_NAME :"
  echo " e_id: $IO_EVNT_ID, e_mask: $IO_EVNT_MSK, flag: $IO_EVNT_FLG, active: $IO_ACT_LEN, period: $IO_PERIOD"
  echo
  echo "Are you sure to set $IO_RULES conditions in ECA for the IO action channel (y/n) ?"
  get_usr_answer
fi

# clean current rules
saft-io-ctl $TR_NAME -x

# configure chosen IO as output
saft-io-ctl $TR_NAME -n $IO_NAME -o 1 -d 0

echo

################ set action rules for IO #######################################

# set ECA rules for chosen output channel
i=1
val=1
offset=0
inactive=$(expr $IO_PERIOD - $IO_ACT_LEN )

while [ $i -le $IO_RULES ]; do

  # active state of a signal
#  echo rule: e_id: $IO_EVNT_ID e_mask: $IO_EVNT_MSK offset: $offset flag: $IO_EVNT_FLG value: $val
  saft-io-ctl $TR_NAME -n $IO_NAME -u -c $IO_EVNT_ID $IO_EVNT_MSK $offset $IO_EVNT_FLG $val

  offset=$( expr $offset + $IO_ACT_LEN )

  if [ $val -eq 0 ]; then
    val=1;
  else
    val=0;
  fi

  i=$( expr $i + 1 )

  if [ $i -gt $IO_RULES ]; then
    break
  fi

  # inactive state
#  echo rule: e_id: $IO_EVNT_ID e_mask: $IO_EVNT_MSK offset: $offset flag: $IO_EVNT_FLG value: $val
  saft-io-ctl $TR_NAME -n $IO_NAME -u -c $IO_EVNT_ID $IO_EVNT_MSK $offset $IO_EVNT_FLG $val

  offset=$( expr $offset + $inactive )

  if [ $val -eq 0 ]; then
    val=1;
  else
    val=0;
  fi

  i=$( expr $i + 1 )

done

# print period of a pulse block
echo "Pulse block period = $offset ( $(printf '0x%x' $offset ) ) ns"
echo

# verify ECA rules for the IO actions
echo Current ECA rules for the IO action channel
saft-io-ctl $TR_NAME -l

echo
