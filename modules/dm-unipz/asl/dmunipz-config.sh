#!/bin/sh
# deployment script: firmware and ECA config

# generic setup
MOUNTPOINT=$1
INFO="$2 $0"
ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

# info
logger "$INFO: start"
logger "$INFO: firmware and ECA configuration"

# specific setup

# execute config script
dm-unipz_start.sh

logger "configure my latest shit $0: done"
