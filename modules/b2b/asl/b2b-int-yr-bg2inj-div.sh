#!/bin/sh
# script for deployment on ASL
#. /etc/functions

logger "$0: initializing"

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

logger "$0: start other stuff"
export DIM_DNS_NODE=asl105
b2b-serv-sys dev/wbm0 -s int_yr-kdi &

logger "$0: done"
