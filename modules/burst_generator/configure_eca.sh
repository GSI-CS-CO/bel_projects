#!/bin/sh
# debug enable
#set -x

echo $#
echo -e "$@\n\n" # new line

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
  echo writing id, delay, conditions, offset and production cycle to RAM:

  local USR_RAM_ADDR=$USR_RAM_START
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 ${IO_EVNT_ID::10}
  printf ' io evnt id @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $PULSE_DELAY
  printf ' delay      @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $IO_RULES
  printf ' conditions @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $IO_EVNT_OFF
  printf ' offset     @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $PULSE_CYCLE_HI32
  printf ' cycle_hi32 @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  USR_RAM_ADDR=$(($USR_RAM_ADDR + 4))
  eb-write dev/wbm0 ${USR_RAM_ADDR}/4 $PULSE_CYCLE_LO32
  printf ' cycle_lo32 @ 0x%08x : 0x%s\n' $USR_RAM_ADDR "$(eb-read dev/wbm0 ${USR_RAM_ADDR}/4)"

  echo
}

############## globals #########################################################

TR_NAME=""
IO_NAME="IO1"

# default number of ECA rules for the IO action channel
IO_RULES=100

# default event offset is 1000ns (1us duty cycle -> 2us period -> 500KHz)
IO_EVNT_OFF=1000

# default event flags (delayed)
IO_EVNT_FLG=0x8

IO_EVNT_ID=0xEEEE000000000000
IO_EVNT_MSK=0xFFFF000000000000

PULSE_DELAY=0;
PULSE_CYCLE_HI32=0
PULSE_CYCLE_LO32=5 # inject timing messages for IO action 5 times

OPTION_WRITE_PULSE_PARAMS_TO_RAM=0
OPTION_USER_INTERACTION=1

if [ $# -ne 0 ]; then
  while getopts ":h:n:f:o:c:d:pu" opt; do
    case $opt in
      h ) # help
        echo "$0 -n <conditions> -o <offset> -f <flags>"
        exit 0 ;;
      n ) # number of ECA conditions (must be even number)
        if [ $(($OPTARG % 2)) ]; then
          IO_RULES=$OPTARG
        else
          echo "$OPTARG must be even number!" 1>&2
          exit 1
        fi ;;
      f ) # flags
        IO_EVNT_FLG=$OPTARG ;;
      o ) # offsets
        IO_EVNT_OFF=$OPTARG ;;
      c ) # pulse cycle high 32-bit
        PULSE_CYCLE_HI32=$OPTARG ;;
      d ) # pulse cycle low 32-bit
        PULSE_CYCLE_LO32=$OPTARG ;;
      p ) # allow to store the pulse params to RAM
        OPTION_WRITE_PULSE_PARAMS_TO_RAM=1 ;;
      u ) # disable user interaction
        OPTION_USER_INTERACTION=0 ;;
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

############# write pulse parameters to shared memory #########################

# write conditions and offset to shared RAM

if [ $OPTION_WRITE_PULSE_PARAMS_TO_RAM -eq 1 ]; then
  USR_RAM_START=0x200a0510
  write_pulse_params_to_ram
fi
############# set action rules for LM32 ########################################

LM32_EVNT_ID=0x8484000000000000
LM32_EVNT_MSK=0xFFFF000000000000
LM32_EVNT_TAG=0x42
LM32_EVNT_OFF=0

if [ $OPTION_USER_INTERACTION -eq 1 ]; then
  echo "Are you sure to set ECA rules for the LM32 action channel:"
  echo " id: $LM32_EVNT_ID, mask: $LM32_EVNT_MSK, offset: $LM32_EVNT_OFF, tag: $LM32_EVNT_TAG (y/n) ?"
  get_usr_answer
fi

#get the name of a timing receiver
TR_NAME=$(saft-ctl bla -f -j | grep -oE "name:(.*?)path" | sed s/,//g | cut -d" " -f2)

# clean current rules
saft-ecpu-ctl $TR_NAME -x

# set ECA rules for the LM32 action channel
saft-ecpu-ctl $TR_NAME -d -c $LM32_EVNT_ID $LM32_EVNT_MSK $LM32_EVNT_OFF $LM32_EVNT_TAG

# verify ECA rules for the LM32 action channel
echo Current ECA rules the LM32 action channel
saft-ecpu-ctl $TR_NAME -l

echo

############## configure IO ####################################################

# list all IO and their capabilities
saft-io-ctl $TR_NAME -i

# choose the first available IO
AVAIL_IO=$(saft-io-ctl $TR_NAME -i | grep '\bOut\b' | head -1 | cut -d" " -f1)
if [ "$AVAIL_IO" != "" ]; then
  IO_NAME=$AVAIL_IO
fi

if [ $OPTION_USER_INTERACTION -eq 1 ]; then
  # ask user agreement prior to configuration
  echo "set $IO_NAME of $TR_NAME as output (y/n) ?"
  get_usr_answer

  echo "Are you sure to set $IO_RULES ECA rules for the IO channel:"
  echo " id: $IO_EVNT_ID, mask: $IO_EVNT_MSK, flag: $IO_EVNT_FLG, offset: $IO_EVNT_OFF (y/n) ?"
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
offset=$IO_EVNT_OFF

while [ $i -le $IO_RULES ]; do

  offset=$( expr $i \* $IO_EVNT_OFF )
#  echo rule: $IO_EVNT_ID mask: $IO_EVNT_MSK offset: $offset flag: $IO_EVNT_FLG value: $val
  saft-io-ctl $TR_NAME -n $IO_NAME -u -c $IO_EVNT_ID $IO_EVNT_MSK $offset $IO_EVNT_FLG $val
  if [ $val -eq 0 ]
  then val=1;
  else val=0; fi

  i=$( expr $i + 1 )
done

# verify ECA rules for the IO actions
echo Current ECA rules for the IO action channel
saft-io-ctl $TR_NAME -l

echo

