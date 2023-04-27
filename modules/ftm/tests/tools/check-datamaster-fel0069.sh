#! /bin/bash

# Monitoring output
echo -n -e '\n\ndev/wbm1 monitoring: '; ssh root@fel0069.acc.gsi.de 'eb-mon -v dev/wbm1'
echo -n 'dev/wbm1 WR time: '; ssh root@fel0069.acc.gsi.de 'eb-mon -d dev/wbm1'
echo -n 'dev/wbm1 host time: '; ssh root@fel0069.acc.gsi.de 'date -Iseconds'
echo -n 'dev/wbm0 datamaster IP: '; ssh root@fel0069.acc.gsi.de 'eb-mon -i dev/wbm0'
echo -n 'dev/wbm0 datamaster WR sync status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -y dev/wbm0'
echo -n 'dev/wbm0 datamaster link status: '; ssh root@fel0069.acc.gsi.de 'eb-mon -l dev/wbm0'
# Test that the tools version and the firmware version are compatible
dm-cmd tcp/fel0069.acc.gsi.de
OPTIONS='-k zzz' make -C ~/bel_projects/dev/modules/ftm/tests remote
