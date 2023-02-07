#!/bin/sh
# dmunipz deployment script: copy libraries and software to FEC

logger "get my latest shit $0: start"

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

logger "$0: copying firmware, software and liraries to ramdisk"
# libraries
#
#ldconfig
# software
cp -a /opt/$NAME/$ARCH/usr/bin/* /usr/bin/
# firmware
cp -a /opt/$NAME/firmware/* /

logger "get my latest shit $0: done"
