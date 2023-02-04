#!/bin/sh
# b2b deployment script: copy libraries and software to FEC

logger "copy stuff from NFS script $0: start"

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

logger "$0: copying firmware, software and liraries to ramdisk"
# libraries
cp -a /opt/$NAME/$ARCH/usr/lib/* /usr/lib/
ldconfig
# software
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
# firmware
cp -a /opt/$NAME/firmware/* /

logger "copy stuff from NFS script $0: done"
