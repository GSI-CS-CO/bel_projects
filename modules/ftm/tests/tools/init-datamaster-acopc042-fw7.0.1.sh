#! /bin/bash

echo 'Check kernel modules and CERN pci devices.'
sudo lspci -nn -vkd 10dc:
echo 'start saft daemon for TR0.'
sudo saftd tr0:$TR0

# set the IP address for the client pexarria5 on acopc042 (no bootp).
# echo -n 'dev/wbm0 set IP: '; echo -n -e '\r'ip set 192.168.129.82'\r\r' | eb-console dev/wbm0;
# switch ptp mode to master and start ptp
# echo -n 'dev/wbm0 set master mode: '; echo -n -e '\r'mode master'\r\r' | eb-console dev/wbm0;
# echo -n 'dev/wbm0 start ptp: '; echo -n -e '\r'ptp start'\r\r' | eb-console dev/wbm0;
# set the time for this ptp master, first seconds, then nanoseconds.
echo -n 'dev/wbm0 set time: '; echo -n -e '\r'time setsec $(date -u +%s)'\r\r' | eb-console dev/wbm0;
echo -n 'dev/wbm0 set time: '; echo -n -e '\r'time setnsec $(date -u +%N)'\r\r' | eb-console dev/wbm0;

# set the IP address for the datamaster test system on acopc042 (no bootp).
# echo -n -e '\n\ndev/wbm1 set IP: '; echo -n -e '\rip set 192.168.128.204\r\r' | eb-console dev/wbm1;
# Monitoring output
echo -n -e '\n\ndev/wbm0 monitoring: '; eb-mon -v dev/wbm0
echo -n 'dev/wbm1 datamaster IP: '; eb-mon -i dev/wbm1
echo -n 'dev/wbm1 datamaster WR sync status: '; eb-mon -y dev/wbm1
# load the latest firmware to datamaster
~/bel_projects/dev/syn/gsi_pexarria5/ftm/fwload_all.sh $DM ~/Documents/fel0069/ftm_v7.0.1.bin
# Test that the tools version and the firmware version are compatible
dm-cmd $DM
OPTIONS='-k zzz' make -C ~/bel_projects/release/modules/ftm/tests
