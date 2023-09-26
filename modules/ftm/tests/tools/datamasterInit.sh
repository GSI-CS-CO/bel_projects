#! /bin/bash
#
# Example: ./datamasterInit.sh $(hostname) $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/ftm.bin

if [ ! $BEL_PROJECTS_PATH ]
then
  BEL_PROJECTS_PATH=$HOME/bel_projects/dev
fi

if [ $# -eq 1 ]
then
  DM_HOST=$1
else
  DM_HOST=$(hostname)
fi

if [ $# -eq 2 ]
then
  FIRMWARE=$2
else
  FIRMWARE=~/Documents/fel0069/ftm_v8.0.4-rc1.bin
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
  # Load the latest firmware to datamaster
  $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/fwload_all.sh $DM $FIRMWARE
  # Test that the tools version and the firmware version are compatible
  LD_LIBRARY_PATH=$BEL_PROJECTS_PATH/modules/ftm/lib/ $BEL_PROJECTS_PATH/modules/ftm/bin/dm-cmd $DM
  OPTIONS='-k zzz' make -C $BEL_PROJECTS_PATH/modules/ftm/tests
elif [ "$DM_HOST" = "fel0069" ]
then
  # echo 'Check kernel modules and CERN pci devices.'
  # sudo lspci -nn -vkd 10dc:
  echo 'Copy ssh ids'
  ssh root@fel0069.acc.gsi.de "mkdir -p ~/.ssh/"
  ssh root@fel0069.acc.gsi.de "tee -a ~/.ssh/authorized_keys" < ~/.ssh/id_ecdsa.pub
  ssh root@fel0069.acc.gsi.de "tee -a ~/.ssh/authorized_keys" < ~/.ssh/id_ecdsa_521_jenkins_vmlb010.pub
  ssh root@fel0069.acc.gsi.de "tee -a ~/.ssh/authorized_keys" < ~/.ssh/id_ecdsa_521_jenkins_tsl021.pub
  echo 'start saft daemon for tr1.'
  ssh root@fel0069.acc.gsi.de '/usr/sbin/saftd tr1:dev/wbm1'
  # Monitoring output
  echo -n -e '\n\ndev/wbm1 monitoring: '; ssh root@fel0069.acc.gsi.de 'eb-mon -v dev/wbm1'
  echo -n 'dev/wbm1   WR time: '; ssh root@fel0069.acc.gsi.de 'eb-mon -d dev/wbm1'
  echo -n 'dev/wbm1 host time: '; ssh root@fel0069.acc.gsi.de 'date -Iseconds'
  echo -n 'dev/wbm0 datamaster IP: '; ssh root@fel0069.acc.gsi.de 'eb-mon -i dev/wbm0'
  echo -n 'dev/wbm0 datamaster WR sync status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -y dev/wbm0'
  echo -n 'dev/wbm0 datamaster link status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -l dev/wbm0'
  # load the latest firmware to datamaster
  $BEL_PROJECTS_PATH/syn/gsi_pexarria5/ftm/fwload_all.sh tcp/fel0069.acc.gsi.de $FIRMWARE
  # Test that the tools version and the firmware version are compatible
  LD_LIBRARY_PATH=$BEL_PROJECTS_PATH/modules/ftm/lib/ $BEL_PROJECTS_PATH/modules/ftm/bin/dm-cmd tcp/fel0069.acc.gsi.de
  OPTIONS='-k zzz' make -C $BEL_PROJECTS_PATH/modules/ftm/tests remote
else
  echo "Unknown datamaster host $DM_HOST, known are ACOPC042, fel0069."
fi
