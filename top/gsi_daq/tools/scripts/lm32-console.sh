# !/bin/bash
###############################################################################
##                                                                           ##
##  Shellscript to launching in a SCU-LM32 console from Linux-homeoffice-PC  ##
##  or directly from ASL-cluster without socat.                              ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-console.sh                                                ##
## Author:    Ulrich Becker                                                  ##
## Date:      06.11.2019                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

EB_CONSOLE=eb-console
WB_DEVICE=dev/wbm0
ADDITIONAL_LIBRARY_PATH="/common/usr/cscofe/lib/"

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
   echo "Tool to launching in a SCU-LM32 console from Linux-home-office-PC" \
        "or directly from ASL-cluster without socat."
   echo
   echo "NOTE: If this script becomes invoked on remote-PC (e.g. home office)" \
        "than a recursive call will made by calling the same script on ASL-cluster."
   echo
   echo "When this script becomes invoked on remote PC, it will ask about the GSI-username."
   echo "The script will skip this question if the environment-variable GSI_USERNAME"
   echo "has been set with your username on your home-PC."
   echo
   echo "Usage: $0 <URL of target SCU>"
   echo "Option:"
   echo "-h  This help"
   exit 0
fi

if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   #
   # Script is running on ASL-cluster
   #
   if [ ! -n "$(which $EB_CONSOLE 2>/dev/null)" ]
   then
      die "$EB_CONSOLE not found on ASL! Check your environment variable PATH on ASL."
   fi
   checkTarget $1
   export LD_LIBRARY_PATH="${ADDITIONAL_LIBRARY_PATH}:${LD_LIBRARY_PATH}"
   echo "Launching of $EB_CONSOLE on $1. Type Ctrl+c to leave." 
   ssh -t root@$1 $EB_CONSOLE $WB_DEVICE
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
