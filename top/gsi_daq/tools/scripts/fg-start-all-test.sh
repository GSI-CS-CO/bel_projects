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

#if [ "${HOSTNAME:0:5}" != "scuxl" ]
#then
#   die "This script can run on SCU only!"
#fi

#if [ ! -n "$1" ]
#then
#   die "Missing fg-wave file name!"
#fi
#if [ ! -f "$1" ]
#then
#   die "No fg-wave file \"$1\" found!"
#fi


#FG_CTL=$(which saft-fg-ctl)
#if [ ! -x "$FG_CTL" ]
#then
#   die "Program saft-fg-ctl not found!"
#fi


#killall daemon 2>/dev/null
#killall $(basename $FG_CTL) 2>/dev/null

#L="Scanning for fg-channels ... trying to own fg fg-39-1 /de/gsi/saftlib/tr0/fg_firmware/fg_2\n"
#L=$L"trying to own fg fg-39-129 /de/gsi/saftlib/tr0/fg_firmware/fg_3\n"
#L=$L"trying to own fg fg-39-130 /de/gsi/saftlib/tr0/fg_firmware/fg_4\n"
#L=$L"trying to own fg fg-4-0 /de/gsi/saftlib/tr0/fg_firmware/fg_0\n"
#L=$L"trying to own fg fg-4-1 /de/gsi/saftlib/tr0/fg_firmware/fg_1"
L="Scanning for fg-channels ... done, found\n"
L=$L"  fg-39-1 /de/gsi/saftlib/tr0/fg_firmware/fg_2\n"
L=$L"  fg-39-129 /de/gsi/saftlib/tr0/fg_firmware/fg_3\n"
L=$L"  fg-39-130 /de/gsi/saftlib/tr0/fg_firmware/fg_4\n"
L=$L"  fg-4-0 /de/gsi/saftlib/tr0/fg_firmware/fg_0\n"
L=$L"  fg-4-1 /de/gsi/saftlib/tr0/fg_firmware/fg_1\n"
L=$L"  fg-12-0 /de/gsi/saftlib/tr0/fg_firmware/fg_1\n"
L=$L"  fg-0-0 /de/gsi/saftlib/tr0/fg_firmware/fg_1\n"

echo -e $L
#exit 0

#FG_LIST=$( $FG_CTL -si 2>/dev/null | egrep 'fg-[0-9]{1,3}-[0-1]' | awk '{printf $1; printf "\n"}')
FG_LIST=$(  echo -e $L | egrep 'fg-[0-9]{1,3}-[0-1]' | awk '{printf $1; printf "\n"}')
sleep 0.5

if [ -n "$2" ]
then
   m=$2
else
   m=10
fi

n=1
for i in $FG_LIST
do
  slot=$(echo $i | tr '-' ' ' | awk '{printf $2}')
  if [ "$slot" -gt "0" ] && [ "$slot" -le "12" ]
  then
     echo -e "activating  $n -> $i"
     #$FG_CTL -rf $i -g <$1 &
     let n+=1
     [ "$n" -gt "$m" ] && break
     sleep 0.5
  else
     echo -e "omitting $i"
  fi
done

echo "Press enter to terminate all running function-generators"
#read key

#killall $(basename $FG_CTL) 2>/dev/null
echo "done"

#=================================== EOF ======================================
