#! /bin/bash -x
set -e
#
# Example: ./datamasterInit.sh $(hostname) $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/ftm.bin

if [ ! $BEL_PROJECTS_PATH ]
then
  # Use the path of the script ${0%/*} and step down.
  BEL_PROJECTS_PATH=${0%/*}/../../../..
fi

# the first argument is the host name.
if [ $# -ge 1 ]
then
  DM_HOST=$1
else
  DM_HOST=$(hostname)
fi

# the second argument is the file of the firmware to be loaded into lm32.
if [ $# -ge 2 ]
then
  FIRMWARE=$2
else
  FIRMWARE=ftm.bin
fi

# the third argument is the number of threads. Values 8 or 32 are allowed.
if [ $# -eq 3 ] && ( [ $3 -eq 8 ] || [ $3 -eq 32 ] )
then
  THR_QTY=$3
else
  THR_QTY=8
fi

if [ "$DM_HOST" = "ACOPC042" ]
then
  echo 'Check kernel modules and CERN pci devices.'
  sudo lspci -nn -vkd 10dc:
  echo 'display info from saft daemon for TR0.'
  saft-ctl -tij tr0
  # set the time for this ptp master, first seconds, then nanoseconds.
  echo -n 'dev/wbm0 set time: '; echo -n -e '\r'time setsec $(date -u +%s)'\r\r' | eb-console dev/wbm0;
  echo -n 'dev/wbm0 set time: '; echo -n -e '\r'time setnsec $(date -u +%N)'\r\r' | eb-console dev/wbm0;
  # Monitoring output
  echo -n -e '\n\ndev/wbm0 monitoring: '; eb-mon -v dev/wbm0
  echo -n 'dev/wbm1 datamaster IP: '; eb-mon -i dev/wbm1
  echo -n 'dev/wbm1 datamaster WR sync status: '; eb-mon -y dev/wbm1
  # Build the latest firmware with 8 or 32 threads
  if [ $# -ge 2 ] && [ ${FIRMWARE} == "ftm.bin" ]
  then
    THR_QTY=$THR_QTY PATH=$PATH:../../../lm32-toolchain/bin/ make -C $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/ $FIRMWARE
    FIRMWARE=$BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/ftm.bin
  fi
  # Load the latest firmware to datamaster
  $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/fwload_all.sh $DM $FIRMWARE
  # Test that the tools version and the firmware version are compatible
  LD_LIBRARY_PATH=$BEL_PROJECTS_PATH/modules/ftm/lib/ $BEL_PROJECTS_PATH/modules/ftm/bin/dm-cmd $DM
  OPTIONS='-k zzz' make -C $BEL_PROJECTS_PATH/modules/ftm/tests
elif [ "$DM_HOST" = "fel0069" ]
then
  # echo 'Check kernel modules and CERN pci devices.'
  # sudo lspci -nn -vkd 10dc:
  if  ssh root@fel0069.acc.gsi.de "saft-ctl tr1 -ij"
  then
    echo 'saft daemon for tr1 started.'
  else
    echo 'check saft daemon for tr1 on fel0069.'
  fi
  # Monitoring output
  echo -n -e '\n\ndev/wbm1 monitoring: '; ssh root@fel0069.acc.gsi.de 'eb-mon -v dev/wbm1'
  echo -n 'dev/wbm1   WR time: '; ssh root@fel0069.acc.gsi.de 'eb-mon -d dev/wbm1'
  echo -n 'dev/wbm1 host time: '; ssh root@fel0069.acc.gsi.de 'date +"%Y-%m-%dT%H:%M:%s%z"'
  echo -n 'dev/wbm0 datamaster IP: '; ssh root@fel0069.acc.gsi.de 'eb-mon -i dev/wbm0'
  echo -n 'dev/wbm0 datamaster WR sync status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -y dev/wbm0'
  echo -n 'dev/wbm0 datamaster link status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -l dev/wbm0'
  # Build the latest firmware with 8 or 32 threads
  if [ $# -ge 2 ] && [ ${FIRMWARE} == "ftm.bin" ]
  then
    THR_QTY=$THR_QTY PATH=$PATH:../../../lm32-toolchain/bin/ make -C $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/ $FIRMWARE
    FIRMWARE=$BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/ftm.bin
  fi
  # load the latest firmware to datamaster
  $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/fwload_all.sh tcp/fel0069.acc.gsi.de $FIRMWARE
  # Test that the tools version and the firmware version are compatible
  LD_LIBRARY_PATH=$BEL_PROJECTS_PATH/modules/ftm/lib/ $BEL_PROJECTS_PATH/modules/ftm/bin/dm-cmd tcp/fel0069.acc.gsi.de
  OPTIONS='-k zzz' make -C $BEL_PROJECTS_PATH/modules/ftm/tests remote
else
  echo "Unknown datamaster host $DM_HOST, known are ACOPC042, fel0069."
fi
