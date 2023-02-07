#!/bin/sh
# dmunipzdeployment script: firmware and ECA config 

logger "configure my latest shit $0: start"

ARCH=$(/bin/uname -m)

logger "$0: firmware and ECA config"

# execute config script
dm-unipz_start.sh

logger "configure my latest shit $0: done"
