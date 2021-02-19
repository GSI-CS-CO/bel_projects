#!/bin/bash

##########################
## Setup of the WR network
##########################
- WRS with dot-config_integration_access_fbas (CONFIG_DOTCONF_INFO="gen_time=2021-02-11+11:42:43;gen_user=timing@tgservice;git_hash=ff32f67-dirty;role=integration_access_fbas;")
- RPi with DHCP/FTP server
- Pexaria as FBASTX, /dev/wbm0
- Pexp as FBASRX, /dev/wbm2

#####################################
## General setup for all timing nodes
#####################################

echo "assign static IP addresses to the timing receivers: 192.168.131.30 for wbm0, 192.168.131.40 for wbm2"
echo "set IP address (wrc): ip set 192.168.131.30"
echo "verify IP address (wrc): ip"
eb-console dev/wbm0
eb-console dev/wbm2

echo "launch the SAFT daemon"
sudo killall saftd > /dev/null

echo "attach 'wbm0' to 'fbastx'"
sudo saftd fbastx:dev/wbm0 fbasrx:dev/wbm2

#echo "attach 'wbm2' to 'fbasrx'"
#saft-ctl fbasrx attach dev/wbm2

echo "get LM32 RAM address (host perspective)"
eb-ls dev/wbm0 | grep LM32
12.14          0000000000000651:10041000           4040000  LM32-CB-Cluster
12.14.5        0000000000000651:54111351           4060000  LM32-RAM-User

######################
## Make 'fbastx' ready
######################

echo "load the LM32 firmware"
eb-fwload dev/wbm0 u 0x0 ~/gsi_prj/bel_projects/modules/fbas/fw/fbastx.bin

# wrc output:
#   fbas0: SHARED_SET_NODETYPE 0x10000694
#   fbas0: SHARED_GET_NODETYPE 0x100006a0
#   fbastx: CPU RAM External 0x 4060000, begin shared 0x00000500
#
#   pSharedCmd = 0x4060508 (CPU RAM External + begin_shared + COMMON_SHARED_CMD)
#   pSharedSetNodeType = 0x4060694 (CPU RAM External +  begin_shared + SHARED_SET_NODETYPE - 0x10000000)
#   pSharedGetNodeType = 0x40606a0 (CPU RAM External +  begin_shared + SHARED_GET_NODETYPE - 0x10000000)

echo "CONFIGURE state "
eb-write dev/wbm0 0x4060508/4 0x1

# wrc output:
#   common-fwlib: received cmd 1
#   fbas0: received unknown command '0x00000001'
#   common-fwlib: changed to state 3

echo "OPREADY state "
eb-write dev/wbm0 0x4060508/4 0x2

# wrc console output:
#   common-fwlib: received cmd 2
#   fbas0: received unknown command '0x00000002'
#   common-fwlib: changed to state 4
#   fbas0: now 1604321478293723256, elap 47781578728

echo "show actual ECA conditions"
saft-ecpu-ctl fbastx -l

echo "destroy all unowned ECA conditions"
saft-ecpu-ctl fbastx -x

echo "disable all events from IO inputs to ECA"
saft-io-ctl fbastx -w

echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x42"
saft-ecpu-ctl fbastx -c 0xffffeeee00000000 0xffffffff00000000 0 0x42 -d

####################
## Make fbasrx ready
####################

echo "assign static IP address to the timing receiver"
eb-console dev/wbm2

echo "load the LM32 firmware"
eb-fwload dev/wbm2 u 0x0 ~/gsi_prj/bel_projects/modules/fbas/fw/fbastx.bin

# wrc output:
#   fbas0: SHARED_SET_NODETYPE 0x10000694
#   fbas0: SHARED_GET_NODETYPE 0x100006a0
#   fbas0: CPU RAM External 0x 4060000, begin shared 0x00000500
#   fbas0: COMMON_SHARED_BEGIN 0x10000500
#
#   pSharedCmd = 0x4060508 (CPU RAM External + begin_shared + COMMON_SHARED_CMD)
#   pSharedSetNodeType = 0x4060694 (CPU RAM External +  begin_shared + SHARED_SET_NODETYPE - 0x10000000)

echo "CONFIGURE state "
eb-write dev/wbm2 0x4060508/4 0x1

echo "set node type to FBASRX (0x1)"
echo "write node type (0x1) to dedicated memory location"
eb-write dev/wbm2 0x4060694/4 0x1

echo "tell LM32 to set the node type"
eb-write dev/wbm2 0x4060508/4 0x15

# wrc output:
#   common-fwlib: common_cmdHandler received unknown command '0x00000015'
#   fbas1: node type 1

echo "verify the actual node type"
eb-read dev/wbm2 0x40606a0/4

# wrc output:
#   00000001

echo "OPREADY state "
eb-write dev/wbm2 0x4060508/4 0x2

# wrc output:
#   common-fwlib: changed to state 4
#   fbas1: now 1604322219371061616, elap 479662071872

echo "show actual ECA conditions"
saft-ecpu-ctl fbasrx -l

echo "destroy all unowned ECA conditions"
saft-ecpu-ctl fbasrx -x

echo "disable all events from IO inputs to ECA"
saft-io-ctl fbasrx -w

echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x24"
saft-ecpu-ctl fbasrx -c 0x1fcafca000000000 0xffffffff00000000 0 0x24 -d

####################
## Test FW operation
####################

echo "injectg timing messages to FBASTX that simulate the FBAS class 2 signals"
saft-ctl fbastx -p inject 0xffffeeee00000000 0x0 1000000

# wrc output (fbas TX):
#   fbas0: ECA action (tag 42, flag 0, ts 1604323496001000000, now 1604323496001004400, poll 4400)
#
# wrc output (fbas RX):
#   fbas1: ECA action (tag 24, flag 0, ts 1604323496001506864, now 1604323496001511016, poll 4152)
#
# transmit time (TX -> RX) = 511016 ns (1604323496001511016−1604323496001000000)


