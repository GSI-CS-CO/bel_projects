#!/bin/sh
# startup script for CBU

# set -x

# lm32 listens to B2B_START message from DM
saft-ecpu-ctl tr0 -c 0x1fff800000000005 0xffffffffffffffff 0 0x800 -d






