#!/bin/sh
# startup script for timing receivers as b2b-pm
#
# pro tip: event snooping via tcpdump on wrs
# - connect a WR port to management port (with media converter in between)
# ==> management port exposed to WR network traffic
# - snoop 'tcpdump host 192.168.0.101 -i eth0 -X' snoops traffice from IP
# set -x


###########################################
# tr0 as extraction machine
###########################################

# IO1 generates TTL for B2B_START event -> cbu
# convenience for triggering scope

#saft-io-ctl tr0 -n IO1 -o 1 -t 0
#saft-io-ctl tr0 -n IO1 -c 0x1fa7800000000000 0xfffffff000000000 0 0x0 1 -u
#saft-io-ctl tr0 -n IO1 -c 0x1fa7800000000000 0xfffffff000000000 10000000 0x0 0 -u

# IO3 configured as TLU input (from 'DDS')
# configure TLU 
saft-io-ctl tr0 -n IO3 -b 0xffff100000000000
#eb-write dev/wbm0 0x4012000/4 0x0004

# lm32 listens to TLU
saft-ecpu-ctl tr0 -c 0xffff100000000001 0xffffffffffffffff 0 0x2 -d

# lm32 listens to B2B_PMEXT message from CBU
saft-ecpu-ctl tr0 -c 0x1fa7801000000000 0xfffffff000000000 0 0x801 -d


# testing pulse upon B2B_DIAGEXT message from CBU
saft-io-ctl tr0 -n IO2 -o 1 -t 0
saft-io-ctl tr0 -n IO2 -c 0x1fa7805000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr0 -n IO2 -c 0x1fa7805000000000 0xfffffff000000000 10000000 0x0 0 -u


###########################################
# tr1 as injection machine
###########################################

# IO1 generates TTL for B2B_DIAGMATCH event
# convenience for triggering scope

saft-io-ctl tr1 -n IO1 -o 1 -t 0
saft-io-ctl tr1 -n IO1 -c 0x1fa7807000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr1 -n IO1 -c 0x1fa7807000000000 0xfffffff000000000 10000000 0x0 0 -u

# IO3 configured as TLU input (from 'DDS')
# configure TLU 
saft-io-ctl tr1 -n IO3 -b 0xffff100000000000
#eb-write dev/wbm1 0x4012000/4 0x0004

# lm32 listens to TLU
saft-ecpu-ctl tr1 -c 0xffff100000000001 0xffffffffffffffff 0 0x2 -d

# lm32 listens to B2B_PMINJ message from CBU
saft-ecpu-ctl tr1 -c 0x1fa7802000000000 0xfffffff000000000 0 0x802 -d


# testing pulse upon B2b_DIAGINJ message from CBU
saft-io-ctl tr1 -n IO2 -o 1 -t 0
saft-io-ctl tr1 -n IO2 -c 0x1fa7806000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr1 -n IO2 -c 0x1fa7806000000000 0xfffffff000000000 10000000 0x0 0 -u


