#!/bin/bash

# start WR network (WRSs, DHCP/FTP server)

# make a timing receiver ready
echo "assign static IP address to the timing receiver"
sudo eb-console dev/wbm0

echo "set IP address (wrc): ip set 192.168.131.30"
echo "verify IP address (wrc): ip"

# make LM32 ready for operation (OPREADY state)
echo "load the LM32 firmware"
sudo eb-fwload dev/wbm0 u 0x0 ~/gsi_prj/bel_projects/modules/fbas/fw/fbastx.bin

echo "fbastx: CPU RAM External 0x 4060000, begin shared 0x00000500"
echo "pSharedCmd = 0x4060508 (pShared + begin_shared + COMMON_SHARED_CMD)"

echo "CONFIGURE state "
sudo eb-write dev/wbm0 0x4060508/4 0x1

# wrc console output:
#   common-fwlib: received cmd 1
#   fbastx: received unknown command '0x00000001'
#   common-fwlib: changed to state 3

echo "OPREADY state "
sudo eb-write dev/wbm0 0x4060508/4 0x2

# wrc console output:
#   common-fwlib: received cmd 2
#   fbastx: received unknown command '0x00000002'
#   common-fwlib: changed to state 4

# setup ECA with the SAFT-tools

echo "terminate running SAFT daemon"
sudo killall saftd > /dev/null

echo "loading SAFT daemon and attach a timing receiver 'tr0' to device 'dev/wbm0'"
sudo saftd tr0:dev/wbm0

echo "show actual ECA conditions"
sudo saft-ecpu-ctl tr0 -l

echo "destroy all unowned ECA conditions"
sudo saft-ecpu-ctl tr0 -x

echo "disable all events from IO inputs to ECA"
sudo saft-io-ctl tr0 -w

echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x42"
sudo saft-ecpu-ctl tr0 -c 0xffffeeee00000000 0xffffffff00000000 0 0x42 -d

# inject a timing message

echo "now test LM32 firmware by injecting timing messages (refer commands given below)"
sudo saft-ctl bla -fp inject 0xffffeeee00000000 0x0 1000000

# wrc console output:
#   fbastx: ECA action (tag 42, ts 1604322672001000000, now 1604322672001004352, elap 4352)

