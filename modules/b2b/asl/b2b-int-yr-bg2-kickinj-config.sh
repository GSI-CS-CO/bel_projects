#!/bin/sh
# b2b deployment script: firmware and ECA config 

logger "hacky unsupported software deployment script $0: start"

ARCH=$(/bin/uname -m)

logger "$0: attach 2nd timing receiver to saftd"
saft-ctl tr1 attach dev/wbm1 

logger "$0: firmware and ECA config"
# copy config script
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-int-yr-bg2-kickinj_start.sh /usr/bin/
# execute config script
b2b-int-yr-bg2-kickinj_start.sh

logger "hacky unsupported software deployment script $0: done"
