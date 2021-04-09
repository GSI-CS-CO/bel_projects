#!/bin/sh
###############################################################################
##                                                                           ##
##    Shellscript to display the build-ID of the LM32-firmware in a SCU      ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-info.sh                                                   ##
## Author:    Ulrich Becker                                                  ##
## Date:      20.11.2020                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"

DEV=dev/wbm0

die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

checkTarget()
{
   [ ! -n "$1" ] && die "Missing target URL!"
   ping -c1 $1 2>/dev/null 1>/dev/null
   [ "$?" != '0' ] && die "Target \"$1\" not found!"
}

if [ "$1" == "-h" ]
then
   echo "Tool to display the build-id of the currently running LM32-firmware" \
        "in a SCU"
   echo "Usage $0 <URL of target SCU>"
   exit 0
fi

if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   #
   # Script is running on ASL-cluster
   #
   checkTarget $1
   echo "Calling eb-info on $1:"
   echo "------------------------"
   ssh root@${1} "eb-info -w $DEV"
   exit $?
else
   #
   # Script ts running on remote-PC
   #
   if [ ! -n "$GSI_USERNAME" ]
   then
      echo "GSI username: "
      read GSI_USERNAME
   fi
   if [ ! -n "$ASL_NO" ]
   then
      ASL_NO=744
   fi
   ASL_URL=asl${ASL_NO}.acc.gsi.de
   echo "Recursive call on asl${ASL_NO}:"
   ssh -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) $1
fi

#=================================== EOF ======================================
