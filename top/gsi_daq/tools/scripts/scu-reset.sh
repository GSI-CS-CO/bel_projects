# !/bin/bash
###############################################################################
##                                                                           ##
##  Wrapper shellscript for using eb-reset from Linux-homeoffice-PC          ##
##  or directly from ASL-cluster.                                            ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      scu-reset.sh                                                   ##
## Author:    Ulrich Becker                                                  ##
## Date:      12.04.2021                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

ASL_TEMP_DIR="/home/bel/${GSI_USERNAME}/lnx/${TEMP_DIR}"
ADDITIONAL_LIBRARY_PATH="/common/usr/cscofe/lib/"
EB_RESET=eb-reset

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

[ -n "$1" ] || die "Missing argument!"

if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   #
   # Script is running on ASL-cluster
   #
   if [ "$1" == "-h" ]
   then
      echo "Wrapper shell script for using $EB_RESET from Linux-homeoffice-PC or directly from ASL-cluster."
      echo "CAUTION! If the SCU-target name has the prefix \"tcp/\" or is omitted than the port forwarder \"socat\""
      echo "         has to be run on SCU. You can accomplish that by calling start-socat.sh"
      echo
      echo "Help of $EB_RESET:"
   fi

   [ -n "$(which $EB_RESET 2>/dev/null)" ] || die "$EB_RESET not found on ASL! Check your environment variable PATH on ASL."

   if [ "${1:0:1}" = "-" ]
   then
      OPTION=$1
      DEVICE=$2
      COMMAND=$3
      PARAM=$4
   else
      OPTION=""
      DEVICE=$1
      COMMAND=$2
      PARAM=$3
   fi

   if [ -n "$DEVICE" ]
   then
      if [ "${DEVICE:0:4}" = "tcp/" ] || [ "${DEVICE:0:4}" = "dev/" ]
      then
         checkTarget ${DEVICE:4:9}
      else
         checkTarget $DEVICE
         DEVICE="tcp/$DEVICE"
      fi
   fi

   if [ "${DEVICE:0:4}" = "tcp/" ] || [ "$1" == "-h" ]
   then
      export LD_LIBRARY_PATH="${ADDITIONAL_LIBRARY_PATH}:${LD_LIBRARY_PATH}"
      $EB_RESET $OPTION $DEVICE $COMMAND $PARAM
   else
      ssh -t root@${DEVICE:4:9} $EB_RESET $OPTION dev/wbm0 $COMMAND $PARAM
   fi
   [ "$?" = "0" ] || die "$EB_RESET failed!"
   echo -e "${ESC_SUCCESS}done${ESC_NORMAL}"
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
   echo "Recursive call on asl${ASL_NO}."
   ssh -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) $1 $2 $3 $4
fi

#=================================== EOF ======================================
