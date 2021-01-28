# !/bin/bash
###############################################################################
##                                                                           ##
##  Script for copying or updating a file in SCU-NFS directories defined in  ##
##                             SCU_TARGET_LIST                               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      public-scu.sh                                                  ##
## Author:    Ulrich Becker                                                  ##
## Date:      28.01.2021                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################

ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"


if [ "$1" == "-h" ]
then
   echo "Script for copying or updating a file in SCU-NFS directories defined in SCU_TARGET_LIST"
   echo
   echo "Usage $0 [-h] <file to copy>"
   echo
   echo "Option:"
   echo "-h  This help"
   echo
   exit 0
fi



if [ ! -n "$SCU_TARGET_LIST" ]
then
   SCU_TARGET_LIST="scuxl0035"
   SCU_TARGET_LIST=$SCU_TARGET_LIST" scuxl0212"
   SCU_TARGET_LIST=$SCU_TARGET_LIST" scuxl0107"
fi

SCU_NFS_BASE_DIR="/common/export/fesa/local/"


die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

if [ "${HOSTNAME:0:5}" != "asl74" ]
then
   die "Starting from wrong host: ${HOSTNAME}"
fi

if [ ! -n "$1" ]
then
   die "Missing filename!"
fi
if [ ! -f "$1" ]
then
   die "Can't find \"$1\"!"
fi

for i in $SCU_TARGET_LIST
do
   echo ${SCU_NFS_BASE_DIR}${i}
   if [ ! -d "${SCU_NFS_BASE_DIR}${i}" ]
   then
      die "NFS-directory \"${SCU_NFS_BASE_DIR}${i}\" doesn't exist!"
   fi
done

for i in $SCU_TARGET_LIST
do
   targetDir="${SCU_NFS_BASE_DIR}${i}/${USER}/"
   mkdir -p $targetDir
   cp -u $1 $targetDir
done

echo "done"

#=================================== EOF ======================================

