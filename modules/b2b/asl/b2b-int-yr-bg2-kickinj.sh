#!/bin/sh
# script for deployment on ASL
#. /etc/functions

logger "$0: initializing"

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

#log 'apply HACK to fix suspicous dynamic library hazard'
#ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

logger "$0: copying software and startup script to ramdisk"
cp -a /opt/$NAME/$ARCH/usr/lib/* /usr/lib/
ldconfig
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-ctl /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-int-yr-bg2-kickinj_start.sh /usr/bin/

logger "$0: copying firmware to ramdisk"
cp -a /opt/$NAME/firmware/* /

logger "$0: starting"
b2b-int-yr-bg2-kickinj_start.sh

logger "$0: done"
