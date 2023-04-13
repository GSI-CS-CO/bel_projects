#! /bin/bash

# echo 'Check kernel modules and CERN pci devices.'
# sudo lspci -nn -vkd 10dc:
echo 'start saft daemon for tr1.'
ssh root@fel0069.acc.gsi.de '/usr/sbin/saftd tr1:dev/wbm1'
echo 'Copy ssh ids'
ssh-copy-id -i ~/.ssh/id_rsa.pub root@fel0069.acc
ssh-copy-id -i ~/.ssh/id_rsa_jenkins.pub root@fel0069.acc

# Monitoring output
echo -n -e '\n\ndev/wbm1 monitoring: '; ssh root@fel0069.acc.gsi.de 'eb-mon -v dev/wbm1'
echo -n 'dev/wbm1 WR time: '; ssh root@fel0069.acc.gsi.de 'eb-mon -d dev/wbm1'
echo -n 'dev/wbm1 host time: '; ssh root@fel0069.acc.gsi.de 'date -Iseconds'
echo -n 'dev/wbm0 datamaster IP: '; ssh root@fel0069.acc.gsi.de 'eb-mon -i dev/wbm0'
echo -n 'dev/wbm0 datamaster WR sync status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -y dev/wbm0'
echo -n 'dev/wbm0 datamaster link status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -l dev/wbm0'
# load the latest firmware to datamaster
~/bel_projects/dev/syn/gsi_pexarria5/ftm/fwload_all.sh tcp/fel0069.acc.gsi.de ~/Documents/fel0069/ftm_v8.0.4-rc1.bin
# Test that the tools version and the firmware version are compatible
dm-cmd tcp/fel0069.acc.gsi.de
OPTIONS='-k zzz' make -C ~/bel_projects/dev/modules/ftm/tests remote
