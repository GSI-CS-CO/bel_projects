#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'apply HACK to fix suspicous dynamic library hazard'
ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

log 'copying software and startup script to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/lib/* /usr/lib/
ldconfig
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-ctl /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-int-na-bg2-pmstub_start.sh /usr/bin/

log 'copying firmware to ramdisk'
cp -a /opt/$NAME/firmware/* /

log 'starting'
b2b-int-na-bg2-pmstub_start.sh
