#!/bin/bash

# pxelinux.cfg = yocto
# /common/export/nfsinit/<target_scu>

target="scuxl0304"

# create timing RTE symlinks
cd /common/export/nfsinit/$target
rm *
ln -s ../global/timing-rte-yocto-loader 10.tg-fallout-v6-3-0-yocto-rc1
ln -s ../global/timing-rte-yocto-loader 20.burst-generator-yocto
ln -s ../global/timing-rte-yocto-loader 29.tg-backdoor-yocto
ln -s ../global/timing-rte-yocto-loader 30.tg-socat-yocto
ls -la

# create ramdisk symlink
cd /common/tftp/csco/pxe/pxelinux.cfg/
rm $target
ln -s yocto $target
ls -la $target
