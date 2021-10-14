#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'start other stuff'
export DIM_DNS_NODE=asl105
b2b-serv-raw tr0 -e0 pro &
b2b-analyzer pro_sis18 &
