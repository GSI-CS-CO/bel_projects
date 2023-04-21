#! /bin/bash

echo 'Check kernel modules and CERN pci devices.'
sudo lspci -nn -vkd 10dc:

# Monitoring output
echo -n -e '\n\ndev/wbm0 monitoring: '; eb-mon -v dev/wbm0
echo -n 'dev/wbm1 datamaster IP: '; eb-mon -i dev/wbm1
echo -n 'dev/wbm1 datamaster WR sync status: '; eb-mon -y dev/wbm1
echo -n 'dev/wbm1 datamaster link status: '; eb-mon -l dev/wbm1
# Test that the tools version and the firmware version are compatible
dm-cmd $DM
OPTIONS='-k zzz' make -C ~/bel_projects/dev/modules/ftm/tests
