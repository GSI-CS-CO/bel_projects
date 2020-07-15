#!/bin/bash
EB_PATH="/common/usr/timing/libs_for_generator_fesa/bel_projects/ip_cores/etherbone-core/api"
LD_LIB="/common/usr/timing/libs_for_generator_fesa/bel_projects/syn/gsi_pexarria5/ftm:$EB_PATH/.libs/libetherbone.so"
EB_PREFIX="LD_LIBRARY_PATH=$LD_LIB"

DIR=`date +'dmpro_dump_%Y%m%d_%H%M%S'`
OWNPATH=${BASH_SOURCE[0]%/*}
DEV="tsl017.acc.gsi.de"

if [ -d "$OWNPATH/$DIR" ]; then
  echo "$OWNPATH/$DIR exists, need a new dump location"
  exit 1
else
  mkdir $OWNPATH/$DIR  
fi
export $EB_PREFIX

echo $OWNPATH/$DIR

#Get adresses for user lm32 rams (Dev ID 54111351)
RAMS=`$EB_PATH/tools/eb-ls tcp/$DEV | grep -Po "(?:54111351\s*)\K(\d+)"`
#memory dump for all rams as 'memdump_<i>.bin'
i=0
for LINE in $RAMS; do
  #TODO: calculate RAM size from difference of first and last address
  $EB_PATH/tools/eb-get tcp/$DEV 0x$LINE/131072 $OWNPATH/$DIR/memdump_$i.bin
  let i+=1
done  

ssh root@$DEV "cd /var/log; dm-cmd dev/wbm0 -v > carpeDMstatus.log; dm-cmd dev/wbm0 details >> carpeDMstatus.log; dm-sched dev/wbm0 -v >> carpeDMstatus.log; dm-sched dev/wbm0 -v -d > carpeDMdebug.log"
scp root@$DEV:/var/log/* $OWNPATH/$DIR/
chown -R :timing $OWNPATH/$DIR
chmod -R g+rw $OWNPATH/$DIR
#EOF
