#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'apply HACK to fix suspicous dynamic library hazard'
ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

log 'copying software to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/bin/phtif /usr/bin/

mkdir /tmp/phtifivt
cp -a /opt/$NAME/$ARCH/tmp/phtifivt/* /tmp/phtifivt

