#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'start other stuff'
export DIM_DNS_NODE=asl105
b2b-serv-sys dev/wbm0 -s sis18-pm &
b2b-serv-sys dev/wbm1 -s sis18-cbu &
