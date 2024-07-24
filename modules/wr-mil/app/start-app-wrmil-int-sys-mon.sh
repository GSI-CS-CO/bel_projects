#!/bin/sh
#set -x

# Absolute path to this script (with resolved symlinks)
SCRIPT=$(readlink -f "$0")
# Absolute path to script base directory  (with resolved symlinks)
SCRIPTDIR=$(dirname "${SCRIPT}")
# We assume that the script resides in bin/ folder inside the base folder, so going one level up
BASEDIR=$(cd "${SCRIPTDIR}/.." >/dev/null; pwd)

# script for starting the wrmil system viewer on INT
export DIM_DNS_NODE=asl105
export LD_LIBRARY_PATH=${BASEDIR}/lib
xterm -T 'wrmil system status' -fa monaco -fs 10 -geometry 173x24 -e ${BASEDIR}/bin/wrmil-client-mon int
