#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'start other stuff'
export DIM_DNS_NODE=lxds014.gsi.de
b2b-serv-raw tr0 -e0 sis18 &
b2b-analyzer sis18 &
