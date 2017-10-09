#!/bin/sh
#...
. /etc/functions

echo $NAME
log 'copying software, tools and startup script to ramdisk'
cp -a /opt/$NAME/$ARCH/usr/bin/* /usr/bin/

log 'copying firmware to ramdisk'
cp -a /opt/$NAME/firmware/* /

log 'starting the gateway'
wrmilgw-startESR.sh | logger -t wrmilgw-startESR -sp local0.info

log 'starting the ESR gateway logger'
wr-mil-gw-ctl dev/wbm0 -m | logger -t wrmilgw-startESR -sp local0.info &