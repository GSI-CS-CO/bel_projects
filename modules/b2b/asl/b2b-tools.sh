#!/bin/sh
# script for deployment on ASL
#. /etc/functions

logger "$0: initializing"

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

#logger 'apply HACK to fix suspicous dynamic library hazard'
#ln -s /usr/lib/libetherbone.so.5 /lib/libetherbone.so.5

logger "$0: copying software to ramdisk"
cp -a /opt/$NAME/$ARCH/usr/lib/* /usr/lib/
ldconfig
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-ui /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-ctl /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-serv-sys /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-client-sys /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-serv-raw /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-analyzer /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-viewer /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/b2b-archiver /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/bin/eb-fwload /usr/bin/

mkdir /tmp/b2bivt
cp -a /opt/$NAME/$ARCH/tmp/b2bivt/* /tmp/b2bivt

logger "$0: done"

