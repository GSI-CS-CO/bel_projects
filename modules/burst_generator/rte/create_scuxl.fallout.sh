#!/bin/bash

# pxelinux.cfg = scuxl.fallout
# /common/export/nfsinit/<target_scu>

ln -s ../global/timing_backdoor 10_timing_backdoor
ln -s ../global/timing-rte-tg-fallout-v6.2.0 20_timing-rte
ln -s ../global/cscohw 70_cscohw
ln -s ../global/timing-rte-tg-socat 90_timing-socat-wbm0
