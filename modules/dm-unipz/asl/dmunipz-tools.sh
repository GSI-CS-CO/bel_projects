#!/bin/sh
# deployment script: copy libraries and software to FEC
# file structure:
# $MOUNTPOINT/<scripts>.sh     , one or many scripts available to nfsinit
# $MOUNTPOINT/firmware/*.bin   , ecpu firmware
# $MOUNTPOINT/systemd/*.service, systemd units
# $MOUNTPOINT/$ARCH/usr/bin    , binaries and scripts for FEC userspace
# $MOUNTPOINT/$ARCH/usr/bin    , shared libraries for FEC userspace

# generic setup
MOUNTPOINT=$1
INFO="$2 $0"
ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

# info
logger "$INFO: start"
logger "$INFO: copying firmware, software and liraries to ramdisk"

# specific setup
# libraries
#
#ldconfig
# software
cp -a $MOUNTPOINT/$ARCH/usr/bin/* /usr/bin/
# firmware
cp -a $MOUNTPOINT/firmware/* /

# info
logger "$INFO: done"
