#!/bin/sh
# startup script for timing receivers as b2b-tdm on tsl404
#
# pro tip: event snooping via tcpdump on wrs
# - connect a WR port to management port (with media converter in between)
# ==> management port exposed to WR network traffic
# - snoop 'tcpdump host 192.168.0.101 -i eth0 -X' snoops traffice from IP
# set -x


###########################################
# tr1 as extraction machine
###########################################

# IO1 generates TTL for B2B_KICKEXT event
saft-io-ctl tr1 -n IO1 -o 1 -t 0
saft-io-ctl tr1 -n IO1 -c 0x1fff808000000000 0xfffffff000000000 0 0x0 1 -u
saft-io-ctl tr1 -n IO1 -c 0x1fff808000000000 0xfffffff000000000 10000000 0x0 0 -u


###########################################
# tr2 as injection machine
###########################################


