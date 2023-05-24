#! /bin/sh

datamasterInfo() {
  echo
  echo " PCI on host $DM_HOST"
  lspci | grep "Class 6800"
  echo 'Check kernel modules and CERN pci devices.'
  sudo lspci -nn -vkd 10dc:
  sudo lspci -vs 0000:04:00.0
  sudo lspci -vs 0000:17:00.0
  ls -l /dev/wb*
  if ! [ -z "$TR0" ]
  then
    echo
    echo
    echo 'Timing-Receiver:' ; eb-mon -vi $TR0 ; eb-info $TR0
  fi
  if ! [ -z "$DM" ]
  then
    echo
    echo
    echo 'Datamaster:' ; eb-info $DM
    echo '$DM datamaster host time: '; date -Iseconds
    echo '$DM datamaster IP: '; eb-mon -i $DM
    echo '$DM datamaster WR sync status: '; eb-mon -y $DM
    echo '$DM datamaster link status: '; eb-mon -l $DM
    echo
    echo ' Datamaster status:'
    dm-cmd $DM -v
  fi
  OPTIONS='-k zzz' make -C ~/bel_projects/dev/modules/ftm/tests
  echo
  echo ' Datamaster log space'
  df -h /var/log/
}

remoteDatamasterInfo() {
  echo
  echo " PCI on host $DM_HOST"
  ssh root@$DM_HOST 'lspci -k | grep "Class 6800"'
  ssh root@$DM_HOST 'ls -l /dev/wb*'
  if ! [ -z "$TR0" ]
  then
    echo
    echo
    echo 'Timing-Receiver:' ; ssh root@$DM_HOST "eb-mon -vi $TR0" ; ssh root@$DM_HOST "eb-info $TR0"
    echo '$TR0 WR time: '; ssh root@$DM_HOST "eb-mon -d $TR0"
  fi
  if ! [ -z "$DM" ]
  then
    echo
    echo
    echo 'Datamaster:' ; ssh root@$DM_HOST "eb-mon -yi $DM" ; ssh root@$DM_HOST "eb-info $DM"
    echo '$DM datamaster host time: '; ssh root@$DM_HOST 'date -Iseconds'
    echo '$DM datamaster IP: '; ssh root@$DM_HOST "eb-mon -i $DM"
    echo '$DM datamaster WR sync status: '; ssh root@$DM_HOST "eb-mon -y $DM"
    echo '$DM datamaster link status: '; ssh root@$DM_HOST "eb-mon -l $DM"
    echo
    echo ' Datamaster status:'
    ssh root@$DM_HOST "dm-cmd -v $DM"
  fi
  # Test that the tools version and the firmware version are compatible
  dm-cmd tcp/$DM_HOST
  #~ OPTIONS='-k zzz' make -C ~/bel_projects/dev/modules/ftm/tests remote
  echo
  echo ' Datamaster log space'
  ssh root@$DM_HOST 'df -h /var/log/'
}

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
elif [ $DM_HOST = 'fel0101.acc' ]
then
  echo "On host $DM_HOST: not implemented"
elif [ $DM_HOST = 'tsl017.acc' ]
then
  echo "On host $DM_HOST: not implemented"
elif [ $DM_HOST = 'tsl018' ]
then
  DM=dev/wbm0
  TR0=""
  if [ "$1" = "remote" ]
  then
    remoteDatamasterInfo
  else
    datamasterInfo
  fi
elif [ $DM_HOST = 'tsl018.acc' ]
then
  DM=dev/wbm0
  TR0=""
  remoteDatamasterInfo
elif [ $DM_HOST = 'tsl020.acc' ]
then
  echo "On host $DM_HOST: not implemented"
else
  echo "Unknown datamaster host $DM_HOST"
fi
