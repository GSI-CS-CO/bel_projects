#! /bin/sh

# Usage:
# ./datamasterCheck.sh [remote] [<hostname>]
# hostname: with correct domain or subdomain.
# If hostname ends with '.acc', argument 'remote' is assumed.
# If no argument is given, the $(hostname) is used.
# If 'remote' keyword is used, ssh is used for eb-mon / eb-info commands
# and dm-cmd is used with socat (tcp/<hostname>).

datamasterInfo() {
  echo
  echo " PCI on host $DM_HOST"
  lspci | grep "Class 6800"
  echo "Check kernel modules and CERN pci devices."
  # this runs with sudo to show the Kernel driver in use.
  sudo lspci -nn -vkd 10dc:
  #~ sudo lspci -vs 0000:04:00.0
  #~ sudo lspci -vs 0000:17:00.0
  ls -l /dev/wb*
  if ! [ -z "$TR0" ]
  then
    echo
    echo
    echo "Timing-Receiver:" ; eb-mon -vi $TR0 ; eb-info $TR0
    saft-ctl -ijf tr
  fi
  if ! [ -z "$DM" ]
  then
    echo
    echo
    echo "Datamaster:" ; eb-info $DM
    echo "$DM datamaster host time: "; date -Iseconds
    echo "$DM datamaster IP: "; eb-mon -i $DM
    echo "$DM datamaster WR sync status: "; eb-mon -y $DM
    echo "$DM datamaster link status: "; eb-mon -l $DM
    echo
    echo " Datamaster status:"
    dm-cmd $DM -v
  fi
  echo " Run one simple test to verify setup."
  if [ ! $BEL_PROJECTS_PATH ]
  then
    # Use the path of the script ${0%/*} and step down.
    BEL_PROJECTS_PATH=${0%/*}/../../../..
  fi
  OPTIONS='-k zzz' make -C $BEL_PROJECTS_PATH/modules/ftm/tests
  echo
  echo " Datamaster log space"
  df -h /var/log/ || true
}

remoteDatamasterInfo() {
  echo
  echo " PCI on host $DM_HOST"
  ssh root@$DM_HOST 'lspci -k | grep "Class 6800"'
  ssh root@$DM_HOST 'ls -l /dev/wb*'
  if [ -z "$TR_HOST" ]
  then
    TR_HOST=$DM_HOST
  fi
  if ! [ -z "$TR0" ]
  then
    echo
    echo
    echo "Timing-Receiver: $TR_HOST" ; ssh root@$TR_HOST "eb-mon -vi $TR0" ; ssh root@$TR_HOST "eb-info $TR0"
    echo "$TR0 WR time: "; ssh root@$TR_HOST "eb-mon -d $TR0"
    ssh root@$TR_HOST "saft-ctl -ijf tr"
  fi
  if ! [ -z "$DM" ]
  then
    echo
    echo
    echo "Datamaster Gateware Image:" ; ssh root@$DM_HOST "eb-info $DM"
    echo "$DM datamaster host time: "; ssh root@$DM_HOST 'date +"%Y-%m-%dT%H:%M:%s%z"'
    echo "$DM datamaster IP: "; ssh root@$DM_HOST "eb-mon -i $DM"
    echo "$DM datamaster WR sync status: "; ssh root@$DM_HOST "eb-mon -y $DM"
    echo "$DM datamaster link status: "; ssh root@$DM_HOST "eb-mon -l $DM"
    echo
    echo " Datamaster status:"
    ssh root@$DM_HOST "dm-cmd -v $DM"
  fi
  # Test that the tools version and the firmware version are compatible
  #~ dm-cmd tcp/$DM_HOST
  #~ OPTIONS='-k zzz' make -C $BEL_PROJECTS_PATH/modules/ftm/tests remote
  echo
  echo " Datamaster log space"
  ssh root@$DM_HOST 'df -h /var/log/' 2>&1 || true
}

# Checking the arguments.
if [ $# -eq 2 ] && [ "$1" = "remote" ]
then
  DM_HOST=$2
elif [ $# -eq 1 ]
then
  DM_HOST=$1
else
  DM_HOST=$(hostname)
fi
echo "Running with datamaster $DM_HOST"

if [ "$DM_HOST" = "ACOPC042" ]
then
  DM=dev/wbm1
  TR0=dev/wbm0
  datamasterInfo
elif [ "$DM_HOST" = "fel0069" ]
then
  DM=dev/wbm0
  TR0=dev/wbm1
  if [ "$1" = "remote" ]
  then
    remoteDatamasterInfo
  else
    datamasterInfo
  fi
elif [ "$DM_HOST" = "fel0069.acc" ]
then
  DM=dev/wbm0
  TR0=dev/wbm1
  remoteDatamasterInfo
elif [ "$DM_HOST" = "tsl014" ]
then
  DM=dev/wbm0
  TR0=dev/wbm0
  TR_HOST=scuxl0001.acc.gsi.de
  if [ "$1" = "remote" ]
  then
    remoteDatamasterInfo
  else
    datamasterInfo
  fi
elif [ "$DM_HOST" = "tsl014.acc" ]
then
  DM=dev/wbm0
  TR0=dev/wbm0
  TR_HOST=scuxl0001.acc.gsi.de
  remoteDatamasterInfo
elif [ "$DM_HOST" = "fel0090.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ "$DM_HOST" = "fel0101.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ "$DM_HOST" = "tsl017.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ "$DM_HOST" = "tsl018" ]
then
  DM=dev/wbm0
  TR0=""
  if [ "$1" = "remote" ]
  then
    remoteDatamasterInfo
  else
    datamasterInfo
  fi
elif [ "$DM_HOST" = "tsl018.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ "$DM_HOST" = "tsl020" ]
then
  DM=dev/wbm0
  TR0=""
  if [ "$1" = "remote" ]
  then
    remoteDatamasterInfo
  else
    datamasterInfo
  fi
elif [ "$DM_HOST" = "tsl020.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ "$DM_HOST" = "fel0077.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ "$DM_HOST" = "fel0006.acc" ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
else
  echo "Unknown datamaster host $DM_HOST"
fi
