#!/bin/bash

# pxelinux.cfg = yocto.scuxl_rt
# /common/export/nfsinit/<target_scu>

ln -s ../global/timing_backdoor 10_timing_backdoor
ln -s ../global/timing-rte-tg-saftlib-dev-yocto 20_timing-rte
ln -s ../global/timing-rte-yocto-loader 21_tg-fallout-v6-2-1-yocto
ln -s ../global/timing-rte-tg-socat 90_timing-socat
