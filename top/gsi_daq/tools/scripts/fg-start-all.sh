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

LM32_CTL="$LM32_CTL tr0"
IO_CTL="$IO_CTL tr0"

killall daemon 2>/dev/null
killall $(basename $FG_CTL) 2>/dev/null

FG_LIST=$( $FG_CTL -si 2>/dev/null | egrep 'fg-[0-9]{1,3}-[0-9]{1,3}' | awk '{printf $1; printf "\n"}')
if [ ! -n "$FG_LIST" ]
then
   die "No function generator(s) found!"
fi


sleep 0.5

if [ -n "$2" ]
then
   m=$2
else
   m=1
fi

n=1
for i in $FG_LIST
do
  slot=$(echo $i | tr '-' ' ' | awk '{printf $2}')
  dev=$(echo $i | tr '-' ' ' | awk '{printf $3}')
  if [ "$slot" -gt "0" ] && [ "$slot" -le "12" ]
  then
     if [ "$dev" -gt "1" ]
     then
        die "Invalid device number $dev for ADDAC-FG on slot $slot !"  
     fi
     echo -e ${ESC_FG_CYAN}"${n}: activating ADDAC-FG ${dev} on slot ${slot}: $i"${ESC_NORMAL}
     $FG_CTL -rf $i -g <$1 &
     sleep 0.5
     let n+=1
     [ "$n" -gt "$m" ] && break
  else
     echo -e ${ESC_FG_BLUE}"omitting $i"${ESC_NORMAL}
  fi
done

echo -e "\n\n*** Press enter to terminate all running function-generators ***"
read key

killall $(basename $FG_CTL) 2>/dev/null
echo "done"

#=================================== EOF ======================================
