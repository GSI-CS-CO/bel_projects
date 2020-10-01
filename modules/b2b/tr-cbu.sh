#!/bin/sh
# startup script for CBU

# set -x

# lm32 listens to B2B_START message from DM
saft-ecpu-ctl tr0 -c 0x1fa7800000000000 0xfffffff000000000 0 0x800 -d

# lm32 listens to B2B_PREXT message from extraction machine
saft-ecpu-ctl tr0 -c 0x1fa7803000000000 0xfffffff000000000 0 0x803 -d

# lm32 listens to B2B_PRINJ message from injecction machine
saft-ecpu-ctl tr0 -c 0x1fa7804000000000 0xfffffff000000000 0 0x804 -d






