#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'apply HACK to fix suspicous dynamic library hazard'
ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

# log 'remove suspicous libc6 and libm6 in /usr/lib (THIS IS A HACK)'
# rm /usr/lib/libc.so.6
# rm /usr/lib/libc-2.17.so
# rm /usr/lib/libm.so.6
# rm /usr/lib/libm-2.17.so

log 'copying software, tools and startup script to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/bin/* /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/lib/* /usr/lib/

ldconfig

log 'copying firmware to ramdisk'
cp -a /opt/$NAME/firmware/* /

# log 'starting monitoring service'
# wrunipz-ctl -s2 dev/wbm0 | logger -t wrunipz-ctl -sp local0.info &

log 'starting the gateway'
wr-unipz_start.sh
