#!/bin/sh
. /etc/functions

log 'initializing'
KERNELVER=$(/bin/uname -r)
ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

# TODO put a list of kernel modules here
MODULES="wishbone pcie_wb vmebus vme_wb"

if [ -f /opt/$NAME/local/$HOSTNAME/conf/sysconfig ]; then
	. /opt/$NAME/local/$HOSTNAME/conf/sysconfig
fi

[ ! -d /lib/modules/$KERNEL_VERSION ] && mkdir -p /lib/modules/$KERNEL_VERSION

if [ ! -d /opt/$NAME/$ARCH/lib/modules/$KERNEL_VERSION ]; then
	log_error "kernel modules for $KERNELVER not available"
fi

log 'copying utilities to ramdisk'
cp -a /opt/$NAME/$ARCH/bin/* /usr/bin/
cp -a /opt/$NAME/$ARCH/sbin/* /usr/sbin/
cp -a /opt/$NAME/$ARCH/share /
cp -a /opt/$NAME/$ARCH/usr/bin/* /usr/bin/
cp -a /opt/$NAME/$ARCH/usr/sbin/* /sbin
cp -a /opt/$NAME/$ARCH/usr/share/* /share
cp -a /opt/$NAME/$ARCH/usr/share/* /usr/share
cp -a /opt/$NAME/$ARCH/usr/include/* /include
cp -a /opt/$NAME/$ARCH/usr/lib/* /lib
cp -a /opt/$NAME/$ARCH/usr/lib64/* /lib
cp -a /opt/$NAME/$ARCH/lib/*.so* /usr/lib
cp -a /opt/$NAME/$ARCH/lib64/*.so* /usr/lib
mkdir /lib/modules/3.10.101-rt111-scu03/extra/
mkdir /lib/modules/4.14.18-rt15-edge01/extra/
mkdir /lib/modules/4.18.16-rt9-scu01/extra
cp -a /opt/$NAME/$ARCH/lib/modules/3.10.101-rt111-scu03/extra/*.ko* /lib/modules/3.10.101-rt111-scu03/extra/
cp -a /opt/$NAME/$ARCH/lib/modules/3.10.101-rt111-scu03/legacy-vme64x-core/drv/driver/*.ko* /lib/modules/3.10.101-rt111-scu03/extra/
cp -a /opt/$NAME/$ARCH/lib/modules/4.14.18-rt15-edge01/extra/*.ko* /lib/modules/4.14.18-rt15-edge01/extra/
cp -a /opt/$NAME/$ARCH/lib/modules/4.14.18-rt15-edge01/legacy-vme64x-core/drv/driver/*.ko* /lib/modules/4.14.18-rt15-edge01/extra/
#cp -a /opt/$NAME/$ARCH/lib/modules/4.18.16-rt9-scu01/extra/*.ko* /lib/modules/4.18.16-rt9-scu01/extra/
#cp -a /opt/$NAME/$ARCH/lib/modules/4.18.16-rt9-scu01/legacy-vme64x-core/drv/driver/*.ko* /lib/modules/4.18.16-rt9-scu01/extra/
cp -a /opt/$NAME/$ARCH/lib/modules/ /lib/modules/4.18.16-rt9-scu01/extra
cp -a /opt/$NAME/$ARCH/etc/* /etc/
cp /opt/$NAME/$ARCH/etc/profile.d/dummy.sh /etc/profile.d/dummy.sh
cp /opt/$NAME/$ARCH/etc/dbus-1/system.conf /etc/dbus-1/system.conf

# super ugly libpthread patch (tbd: clean up this script)
cp ./lib/libpthread-2.17.so ./usr/lib/libpthread-2.17.so

# bash/ash patch (needed since RAM disk from July 2021)
sed -i 's/bash/ash/g' ./usr/bin/eb-flash-secure

# check if we are running admin mode
if [ -f /etc/admin ]; then
	log 'locking device'
	killall dropbear
	sleep 1
	# disable password logins
	dropbear -s -B
fi

# run ldconfig
ldconfig

# load drivers
insmod /lib/modules/$KERNEL_VERSION/extra/wishbone.ko
insmod /lib/modules/$KERNEL_VERSION/extra/pcie_wb.ko
insmod /lib/modules/$KERNEL_VERSION/extra/vmebus.ko
insmod /lib/modules/$KERNEL_VERSION/kernel/drivers/usb/serial/usbserial.ko
insmod /lib/modules/$KERNEL_VERSION/extra/wishbone-serial.ko

# start etherbone TCP->PCIe gateway
#test -f /usr/bin/socat || cp -a /opt/$NAME/socat /usr/bin
#/usr/bin/socat tcp-listen:60368,reuseaddr,fork file:/dev/wbm0 &

# vme_wb driver is loaded if vmebus is already loaded
if [ `ls /proc/vme/info | grep -o info` ]
then
	# /sbin/rmmod pcie_wb
	# Load different slot numbers for "automatic" card detection up to slot 8 (loading the vme_wb driver with non-existing slots does not harm)
	/sbin/insmod /lib/modules/$KERNEL_VERSION/extra/modules/extra/vme_wb.ko slot=1,2,3,4,5,6,7,8 vmebase=0,0,0,0,0,0,0,0 vector=1,2,3,4,5,6,7,8 level=7,7,7,7,7,7,7,7 lun=1,2,3,4,5,6,7,8
fi

log 'copying firmware to ramdisk'
cp -a /opt/$NAME/firmware/* /

log 'copying test artifacts to ramdisk'
cp -a /opt/$NAME/test/* /

log 'starting services'
# start saftlib for multiple devices: saftd tr0:dev/wbm0 tr1:dev/wbm1 tr2:dev/wbm2 ... trXYZ:dev/wbmXYZ
saftlib_devices=$(for dev in /dev/wbm*; do echo tr${dev#/dev/wbm}:${dev#/}; done)
chrt -r 25 saftd $saftlib_devices >/tmp/saftd.log 2>&1 &
# disable the watchdog timer
# for dev in /dev/wbm*; do eval eb-reset ${dev#/} wddisable; done
# reset statistics for eCPU stalls and WR time
for dev in /dev/wbm*; do eval eb-mon ${dev#/} wrstatreset 8 50000; done

# create version file
cat /etc/timing-rte_buildinfo | grep BEL_PROJECTS -A 2 | tail -n 1 | cut -d "-" -f 3 | cut -d "@" -f1 | cut -c 2- > /etc/timing-rte_version

# publish timing receiver info
trinfofpga=$(echo "trinfo FPGA:"; for dev in /dev/wbm*; do eb-info ${dev#/} | grep "FPGA model"; echo "; "; eb-info ${dev#/} | grep "Platform"; echo "; "; eb-info ${dev#/} | grep "Build type"; echo "; uptime [h]"; eb-mon ${dev#/} -z; done)
echo $trinfofpga | log

trinfowr=$(echo "trinfo WR NIC:"; for dev in /dev/wbm*; do echo "MAC:"; eb-mon ${dev#/} -m; echo "; IP:"; eb-mon ${dev#/} -i; echo "; Ethernet: "; eb-mon ${dev#/} -l; echo "; WR-PTP: "; eb-mon ${dev#/} -y; done)
echo $trinfowr | log

trinforte=$(echo "trinfo RTE:"; cat /etc/timing-rte_version; echo "@ramdisk: "; cat /etc/os-release)
echo $trinforte | log
