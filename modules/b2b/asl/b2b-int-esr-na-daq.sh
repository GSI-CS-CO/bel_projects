#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'start other stuff'
export DIM_DNS_NODE=asl105
b2b-serv-raw tr1 -e1 int &
b2b-analyzer int_esr &
