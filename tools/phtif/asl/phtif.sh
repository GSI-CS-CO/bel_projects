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

log 'copying ivtpar files to ramdisk'
mkdir /tmp/phtifivt
cp -a /opt/$NAME/$ARCH/tmp/phtifivt/* /tmp/phtifivt
chmod a+w -R /tmp/phtifivt/

#log 'creating phtif user'
#touch /etc/phtif

#adduser --disabled-password --shell /bin/sh --home / phtif
#echo "phtif:phtif" | chpasswd

# there is an issue with user phtif we can't fix; lets be nasty
log 'change pw to phtif'
echo "root:phtif" | chpasswd
