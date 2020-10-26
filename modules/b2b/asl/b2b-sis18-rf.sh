#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'apply HACK to fix suspicous dynamic library hazard'
ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

log 'copying software, tools and startup script to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/bin/* /usr/bin/

log 'copying firmware to ramdisk'
cp -a /opt/$NAME/firmware/* /

log 'starting the b2b-sis18-rf'
b2b-sis18-rf_start.sh
