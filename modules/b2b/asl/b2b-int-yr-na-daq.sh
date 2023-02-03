#!/bin/sh
# script for deployment on ASL
#. /etc/functions

logger "$0: initializing"

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

logger "$0: start other stuff"
export DIM_DNS_NODE=asl105
b2b-serv-raw tr0 -e2 int &
b2b-analyzer int_yr &

ps | grep b2b | logger

logger "$0: done"
