# !/bin/bash
###############################################################################
##                                                                           ##
##           Starter script for all detected function generators             ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      fg-start-all.sh                                                ##
## Author:    Ulrich Becker                                                  ##
## Date:      26.01.2021                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
PROG_NAME=${0##*/}
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"
ESC_FG_CYAN="\e[1m\e[36m"
ESC_FG_BLUE="\e[34m"

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

#------------------------------------------------------------------------------
isInArgument()
{
   for i in $ACTIVATION_LIST
   do
      if [ "$i" = "$1" ]
      then
         echo true
         return
      fi
   done
   echo false
}

#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Starter script for all detected function generators on this SCU.

Usage:  $PROG_NAME [OPTION] <wave file> <FG channel numbers>
Author: Ulrich Becker

Options:
-h, --help 
   This help and exit.

-e, --exit
   Exit after activation of the function generators.

-n, --nokill
   Don't kill already running fg-ctl instances.

-s, --strobe
   Generates a 10 us trigger strobe on LEMO.

-t, --tag 
   Generates a start tag for all function generator channels else
   for the last channel only.

Example:
$PROG_NAME sinus.fgw 1 3 5
Funktion generator channels 1, 2 and 5 will activated if present.

__EOH__
   exit 0
}

#==============================================================================

if [ "${HOSTNAME:0:5}" != "scuxl" ]
then
   die "This script can run on SCU only!"
fi

FG_CTL=$(which saft-fg-ctl)
if [ ! -x "$FG_CTL" ]
then
   die "Program saft-fg-ctl not found!"
fi

LM32_CTL=$(which saft-ecpu-ctl)
if [ ! -x "$LM32_CTL" ]
then
   die "Program saft-ecpu-ctl not found!"
fi

IO_CTL=$(which saft-io-ctl)
if [ ! -x "$IO_CTL" ]
then
   die "Program saft-io-ctl not found!"
fi

WB_WRITE=$(which eb-write)
if [ ! -x "$WB_WRITE" ]
then
   die "Program eb-write not found!"
fi

SAFT_CTL=$(which saft-ctl)
if [ ! -x "$SAFT_CTL" ]
then
   die "Program saft-ctl not found!"
fi

#
# Parsing of all options:
#
GENERATE_STROBE=false
DO_EXIT=false
NO_KILL=false
while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
         "e")
            DO_EXIT=true
         ;;
         "h")
            printHelp
         ;;
         "n")
            NO_KILL=true
         ;;
         "s")
            GENERATE_STROBE=true 
         ;;
         "t")
            START_TAG="-g"
         ;;
         "-")
            B=${A#*-}
            case ${B%=*} in
               "help")
                  printHelp
               ;;
               "exit")
                  DO_EXIT=true
               ;;
               "nokill")
                  NO_KILL=true
               ;;
               "strobe")
                  GENERATE_STROBE=true 
               ;;
               "tag")
                  START_TAG="-g"
               ;;
               *)
                  die "Unknown long option \"-${A}\"!"
               ;;
            esac
            A=""
         ;;
         *)
            die "Unknown short option: \"${A:0:1}\"!"
         ;;
      esac
      A=${A#?}
   done
   shift
done

WAVE_FILE=$1
shift

if [ ! -n "$WAVE_FILE" ]
then
   die "Missing fg-wave file name!"
fi
if [ ! -f "$WAVE_FILE" ]
then
   die "No fg-wave file \"$WAVE_FILE\" found!"
fi

#
# Checks whether a FESA demon is running.
#
if [ -n "$(pidof daemon)" ]
then
   echo "CAUTION: FESA daemon is running! If you will run this script then the FESA-daemon will terminate!"
   read -r -p "Are you sure? [y/N] " response
   case "$response" in
      [yY][eE][sS]|[yY])
         echo -e ${ESC_FG_CYAN}"Killing daemon..."${ESC_NORMAL}
         killall daemon 2>/dev/null
         sleep 1
   ;;
      *)
         exit 0
   ;;
   esac
fi

#
# Calculates the number of fg-channels which shall be activated.
#
ACTIVATION_LIST=""
while [ -n "$1" ]
do
   ACTIVATION_LIST="$ACTIVATION_LIST $1"
   shift
done

OUT="tr0"
LM32_CTL="$LM32_CTL $OUT"
IO_CTL="$IO_CTL $OUT"
SAFT_CTL="$SAFT_CTL $OUT"
WB_WRITE="$WB_WRITE dev/wbm0"

#
# Kills possible already running instances of saft-fg-ctl.
#
if [ -n "$(pidof $(basename $FG_CTL))" ] && ! $NO_KILL
then
   echo -e ${ESC_FG_CYAN}"Killing running saft-fg-ctl applications..."${ESC_NORMAL}
   killall $(basename $FG_CTL) 2>/dev/null
   sleep 1
fi

#
# Initializing of the FG-list by all found function generators..
#
FG_LIST=$( $FG_CTL -si 2>/dev/null | egrep 'fg-[0-9]{1,3}-[0-9]{1,3}' | awk '{printf $1; printf "\n"}')
if [ ! -n "$FG_LIST" ]
then
   die "No function generator(s) found!"
fi

#
# Calculates the number of found function-generators and the number of function generators
# which shall be activated.
#
NUM_OF_FG=0
MAX_FG_TO_ACTIVATE=0
for i in $FG_LIST
do
   let NUM_OF_FG+=1
   if [ $(isInArgument $NUM_OF_FG) == "true" ]
   then
      let MAX_FG_TO_ACTIVATE+=1
   fi
done
echo -e ${ESC_FG_CYAN}"In total $NUM_OF_FG function generators found on $HOSTNAME."${ESC_NORMAL}
echo -e ${ESC_FG_CYAN}"To activate: $MAX_FG_TO_ACTIVATE function generators."${ESC_NORMAL}

MIL_COUNT=0
n=0
activated=0
lastSlot=-1
#
# For all found function generators:
#
for i in $FG_LIST
do
   let n+=1
   socket=$(echo $i | tr '-' ' ' | awk '{printf $2}')
   slot=$(( socket & 0x000F ))
   dev=$(echo $i | tr '-' ' ' | awk '{printf $3}')
   if [ $(isInArgument $n) == "false" ]
   then
     if [ "$slot" -eq "$socket" ]
     then
        echo -e ${ESC_FG_BLUE}"${n}: omitting ADDAC-FG: ${dev} on slot ${slot}: $i"${ESC_NORMAL}
     else
        echo -e ${ESC_FG_BLUE}"${n}: omitting MIL-FG: ${dev} on slot ${slot}: $i"${ESC_NORMAL}
     fi
     continue 
   fi
   let activated+=1
   #
   # Is ADDAC device?
   #
   if [ "$slot" -eq "$socket" ]
   then
      #
      # ADDAC-device (respectively scu-bus fg)
      #
      if [ "$dev" -gt "1" ]
      then
         die "Invalid device number $dev for ADDAC-FG on slot $slot !"  
      fi
      echo -e ${ESC_FG_CYAN}"${n}: activating ADDAC-FG ${dev} on slot ${slot}: $i"${ESC_NORMAL}
   else
      #
      # MIL device
      #
      if [ "$slot" -ge "1" ]
      then
         #
         # MIL over SIO (SCU-bus)
         #
         if [ "$lastSlot" -ne "$slot" ]
         then
            echo -e ${ESC_FG_CYAN}"Sending broadcast and clearing power up bit of SIO on slot ${slot}."${ESC_NORMAL}
            slaveBase=$((slot*0x20000+0x400800))
            $WB_WRITE $(printf "0x%X/2" $slaveBase) 0x100
            $WB_WRITE $(printf "0x%X/2" $((slaveBase+2))) 0x12FF
         fi
         echo -e ${ESC_FG_CYAN}"${n}: activating SIO-MIL-FG ${dev} on slot ${slot}: $i"${ESC_NORMAL}
      else
         #
         # MIL over extension
         #
         if [ "$lastSlot" -ne "$slot" ]
         then
            echo -e ${ESC_FG_CYAN}"Sending broadcast and clearing power up bit of MIL-extension."${ESC_NORMAL}
            $WB_WRITE 0x9000/4 0x100
            $WB_WRITE 0x9004/4 0x12FF
         fi
         echo -e ${ESC_FG_CYAN}"${n}: activating extension-MIL-FG ${dev} on socket ${socket}: $i"${ESC_NORMAL}
      fi
      lastSlot=$slot
      let MIL_COUNT+=1
   fi
   if [ "$activated" = "$MAX_FG_TO_ACTIVATE" ]
   then
      #
      # Start of the last function generator in list by generating start-tag.
      #
      $FG_CTL -rf $i -g <$WAVE_FILE &
   else
      #
      # Start of a function generator in list.
      #
      $FG_CTL -rf $i $START_TAG <$WAVE_FILE &
   fi
   #sleep 0.5
done
#
# End for all found function generators
#

if [ "$MAX_FG_TO_ACTIVATE" = "0" ]
then
   exit 0
fi

if [ "$MIL_COUNT" -gt "0" ]
then
   echo -e ${ESC_FG_CYAN}"*** $MIL_COUNT MIL-FGs activated. Creating action for LM32. ***"${ESC_NORMAL}
   $LM32_CTL -x
   $LM32_CTL -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFF 0x0 0xDEADBEEF -d
fi

if $GENERATE_STROBE
then
   echo -e ${ESC_FG_CYAN}"Generating 10 us trigger strobe on LEMO."${ESC_NORMAL}
   $IO_CTL -x
   $IO_CTL -n B1 -o1
   $IO_CTL -n B1 -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFFFFFF 0 0x0 0x1 -u
   $IO_CTL -n B1 -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFFFFFF 10000 0x0 0x0 -u
fi

if [ "$MIL_COUNT" -gt "0" ]
then
   $SAFT_CTL inject 0xCAFEBABE 0xFFFFFFFFFFFFFFFF 10000 -p
fi

if $DO_EXIT
then
   exit 0;
fi

echo
read -p "$(echo -e "*** Press enter to terminate all running function-generators ***\n\n")"

echo -e ${ESC_FG_CYAN}"Terminating all running function generators."${ESC_NORMAL}
killall $(basename $FG_CTL) 2>/dev/null

if [ "$MIL_COUNT" -gt "0" ]
then
   echo -e ${ESC_FG_CYAN}"Clearing action for LM32."${ESC_NORMAL} 
   $LM32_CTL -x
   $IO_CTL -x
fi

echo "done"
#=================================== EOF ======================================
