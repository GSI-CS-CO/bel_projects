#!/bin/sh
#...
. /etc/functions

echo $NAME
log 'copying software, tools and startup script to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/bin/* /usr/bin/

log 'copying firmware to ramdisk'
cp -a /opt/$NAME/firmware/* /

log 'starting the gateway'
wrmilgw-startSIS.sh | logger -t wrmilgw-startSIS -sp local0.info

log 'starting the SIS gateway logger'
wr-mil-gw-ctl dev/wbm0 -M SIS18 | logger -t wrmilgw-SIS18 -sp local0.info &
