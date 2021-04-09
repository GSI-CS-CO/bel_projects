#!/bin/sh
###############################################################################
##                                                                           ##
## Shellscript to activate / deactivate the port forwarder "socat" in a SCU  ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      start-socat.sh                                                 ##
## Author:    Ulrich Becker                                                  ##
## Date:      13.11.2019                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

PORT=60368
DEV=/dev/wbm0

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
   echo "Tool for activating or deactivating the port forwarder \"socat\" to the" \
        " whisbone master \"${DEV}\" in a SCU"
   echo "Usage $0 [-k] <URL of target SCU>"
   echo "Option:"
   echo "-k  Deactivating of socat"
   exit 0
fi

if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   #
   # Script is running on ASL-cluster
   #
   if [ "$1" == "-k" ]
   then
      checkTarget $2
      ssh root@${2} "killall socat"
      [ "$?" == "0" ] && echo -e $ESC_SUCCESS"socat stopped on \"${2}\"!"$ESC_NORMAL
      exit $?
   fi

   checkTarget $1

   ssh root@${1} "$(which socat) tcp-listen:$PORT,reuseaddr,fork file:$DEV </dev/null &"
   [ "$?" == "0" ] && echo -e $ESC_SUCCESS"socat started on \"${1}\"!"$ESC_NORMAL
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
   ssh -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) $1 $2
fi

#=================================== EOF ======================================
