# !/bin/bash
###############################################################################
##                                                                           ##
##  Shellscript to uploading LM32-firmware from Linux-homeoffice-PC          ##
##  or directly from ASL-cluster.                                            ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-fwload.sh                                                 ##
## Author:    Ulrich Becker                                                  ##
## Date:      09.04.2021                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

TEMP_DIR="tmp"
ASL_TEMP_DIR="/home/bel/${GSI_USERNAME}/lnx/${TEMP_DIR}"
ADDITIONAL_LIBRARY_PATH="/common/usr/cscofe/lib/"
EB_FWLOAD=eb-fwload
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

checkFile()
{
   [ -f "$1" ] || die "File \"$1\" not found!"
   [ ${1: -4} == ".bin" ] || die "\"$(basename $1)\" has a wrong file format!"
}

cleanTempFile()
{
   if [ "$2" == "remote" ]
   then
      echo "Removing file \"$1\"."
      rm ${1}
   fi
}

[ -n "$1" ] || die "Missing argument!"

if [ "$1" == "-h" ]
then
   echo "Shellscript to uploading LM32-firmware from Linux-homeoffice-PC or directly from ASL-cluster."
   echo
   echo "Usage: $0 [-h] <LM32- firmware binary file> <URL of target SCU>"
   echo "Option:"
   echo "-h  This help"
   echo
   echo "CAUTION: If this script will invoked outside of the ASL-cluster then the folder \"~/$TEMP_DIR\""
   echo "         is prerequisite on the base of your ASL- home directory."
   echo
   exit 0
fi

[ "${1:0:1}" != "-" ] || die "Unknown option: \"$1\"!"


if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   #
   # Script is running on ASL-cluster
   #
   L="$EB_FWLOAD $EB_RESET"
   for i in $L
   do
      [ -n "$(which $i 2>/dev/null)" ] || die "$i not found on ASL! Check your environment variable PATH on ASL."
   done

   checkTarget $2
   checkFile $1
   export LD_LIBRARY_PATH="${ADDITIONAL_LIBRARY_PATH}:${LD_LIBRARY_PATH}"

   $EB_RESET  tcp/${2} cpuhalt 0
   if [ "$?" != "0" ]
   then
      cleanTempFile $1 $3
      die "$EB_RESET failed!"
   fi

   $EB_FWLOAD tcp/${2} u0 0 ${1}
   res=$?
   cleanTempFile $1 $3
   [ "$res" != "0" ] && die "$EB_FWLOAD: Upload of \"$1\" to $2 failed!"
   echo -e "${ESC_SUCCESS}Firmware \"$1\" has been uploaded to $2.${ESC_NORMAL}"
else
   #
   # Script ts running on remote-PC
   #
   checkFile $1
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
   echo "Copy file \"$1\" to asl${ASL_NO}:${ASL_TEMP_DIR}"
   scp $1 ${GSI_USERNAME}@${ASL_URL}:${ASL_TEMP_DIR}/ 1>/dev/null
   [ "$?" != "0" ] && die "Could not copy file \"$1\"!"
   echo "Recursive call on asl${ASL_NO}."
   ssh -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) ${ASL_TEMP_DIR}/$(basename $1) ${2} "remote"
fi

#=================================== EOF ======================================
