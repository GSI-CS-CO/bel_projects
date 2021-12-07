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
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"
ESC_FG_CYAN="\e[1m\e[36m"
ESC_FG_BLUE="\e[34m"


die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

if [ "$1" == "-h" ]
then
   echo "Starter script for all detected function generators"
   echo
   echo "Usage $0 [-h] <fg-wave-file> [maximum number of function generators to start]"
   echo
   echo "If the second optional parameter omited then only the first found" \
        "function generator becones started."
   echo
   echo "Option:"
   echo "-h  This help"
   echo
   exit 0
fi

if [ "${HOSTNAME:0:5}" != "scuxl" ]
then
   die "This script can run on SCU only!"
fi

if [ ! -n "$1" ]
then
   die "Missing fg-wave file name!"
fi
if [ ! -f "$1" ]
then
   die "No fg-wave file \"$1\" found!"
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


OUT="tr0"

LM32_CTL="$LM32_CTL $OUT"
IO_CTL="$IO_CTL $OUT"
SAFT_CTL="$SAFT_CTL $OUT"
WB_WRITE="$WB_WRITE dev/wbm0"

if [ -n "$(pidof daemon)" ]
then
   echo "CAUTION: FESA daemon is running! If you will run this script then the FESA-daemon will terminate!"
   read -r -p "Are you sure? [y/N] " response
   case "$response" in
      [yY][eE][sS]|[yY])
         echo -e ${ESC_FG_BLUE}"Killing daemon..."${ESC_NORMAL}
         killall daemon 2>/dev/null
         sleep 1
   ;;
      *)
         exit 0
   ;;
   esac
fi

if [ -n "$(pidof $(basename $FG_CTL))" ]
then
   echo -e ${ESC_FG_BLUE}"Killing running saft-fg-ctl applications..."${ESC_NORMAL}
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

NUM_OF_FG=0
for i in $FG_LIST
do
   let NUM_OF_FG+=1
done
echo -e ${ESC_FG_CYAN}"In total $NUM_OF_FG function generators found.\n"${ESC_NORMAL}

#sleep 0.5

if [ -n "$2" ]
then
   m=$2
else
   m=1
fi

MIL_COUNT=0
n=1
lastSlot=-1
#
# For all found function generators:
#
for i in $FG_LIST
do
   socket=$(echo $i | tr '-' ' ' | awk '{printf $2}')
   slot=$(( socket & 0x000F ))
   dev=$(echo $i | tr '-' ' ' | awk '{printf $3}')
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
   let n+=1
   if [ "$n" -gt "$m" ]
   then
      #
      # Start of the last function generator in list by generating start-tag.
      #
      $FG_CTL -rf $i -g <$1 &
   else
      #
      # Start of a function generator in list.
      #
      $FG_CTL -rf $i <$1 &
   fi
   #sleep 0.5
   [ "$n" -gt "$m" ] && break
done
#
# End for all found function generators
#


if [ "$MIL_COUNT" -gt "0" ]
then
   echo -e ${ESC_FG_CYAN}"*** $MIL_COUNT MIL-FGs activated. Creating action for LM32. ***"${ESC_NORMAL}
   $LM32_CTL -x
   $LM32_CTL -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFF 0x0 0xDEADBEEF -d
   
   echo -e ${ESC_FG_BLUE}"Generating 10 us trigger strobe on LEMO."${ESC_NORMAL}
   $IO_CTL -x
   $IO_CTL -n B1 -o1
   $IO_CTL -n B1 -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFFFFFF 0 0x0 0x1 -u
   $IO_CTL -n B1 -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFFFFFF 10000 0x0 0x0 -u

   $SAFT_CTL inject 0xCAFEBABE 0xFFFFFFFFFFFFFFFF 10000 -p
fi

echo
echo
read -p "$(echo -e "*** Press enter to terminate all running function-generators ***\n\n")"

echo -e ${ESC_FG_BLUE}"Terminating all running function generators."${ESC_NORMAL}
killall $(basename $FG_CTL) 2>/dev/null

if [ "$IS_MIL" == true ]
then
   echo -e ${ESC_FG_BLUE}"Clearing action for LM32."${ESC_NORMAL} 
   $LM32_CTL -x
   $IO_CTL -x
fi

echo "done"

#=================================== EOF ======================================
