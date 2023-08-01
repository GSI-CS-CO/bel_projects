#!/bin/bash

# Assume: remote device (SCU3) runs yocto ramdisk (with systemctl)!

target='scuxl0304.acc.gsi.de'
remote_cmd='systemctl stop saftbusd && eb-fwload dev/wbm0 u 0 /usr/share/saftlib/firmware/burstgen.bin'

pid_saftd=$(pidof saftd)
pid_saftbusd=$(pidof saftbusd)

if [ -n "$pid_saftd" ]; then
    echo "Warning: saftd is running. Close it manually. Exit!"
    exit 1
fi

if [ -n "$pid_saftbusd" ]; then
    echo "Warning: saftbusd is running. Closing it.."
    saftbus-ctl --quit
fi

if [ -z "$SAFTBUS_SOCKET_PATH" ]; then
    export SAFTBUS_SOCKET_PATH=/tmp/saftbus
fi

ssh root@$target "$remote_cmd"
saftbusd libsaft-service.so tr0:tcp/$target &
saftbus-ctl -l libbg-firmware-service.so tr0
