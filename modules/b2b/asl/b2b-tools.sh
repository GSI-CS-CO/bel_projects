#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'apply HACK to fix suspicous dynamic library hazard'
ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

log 'copying software to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/lib/* /usr/lib/
ldconfig
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-ui /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-ctl /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-serv-sys /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-client-sys /usr/bin/

mkdir /tmp/b2bivt
cp -a /opt/$NAME/$ARCH/tmp/b2bivt/* /tmp/b2bivt

