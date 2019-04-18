#!/bin/sh
# startup script for tsl022

# set -x



###########################################
# tr0 as source machine
###########################################

# IO1 generates TTL for PHASE event
# convenience for triggering scope

saft-io-ctl tr0 -n IO1 -o 1 -t 0
saft-io-ctl tr0 -n IO1 -c 0x1aaabbb000000005 0xffffffffffffffff 0 0x0 1 -u
saft-io-ctl tr0 -n IO1 -c 0x1aaabbb000000005 0xffffffffffffffff 10000000 0x0 0 -u

# IO3 configured as TLU input (from DDS)
# configure TLU (input B1, on rising edge, it  will generate action with tag 0x2
saft-io-ctl tr0 -n IO3 -b 0xffff100000000000
eb-write dev/wbm0 0x4012000/4 0x0004

# lm32 listens to TLU
saft-ecpu-ctl tr0 -c 0xffff100000000004 0xffffffffffffffff 0 0x3 -d

# lm32 listens to PHASE message from DM
saft-ecpu-ctl tr0 -c 0x1aaabbb000000005 0xffffffffffffffff 0 0x2 -d



# testing pulse upon phase message from TR source
saft-io-ctl tr1 -n IO2 -o 1 -t 0
saft-io-ctl tr1 -n IO2 -c 0x0123456789abcdef 0xffffffffffffffff 0 0x0 1 -u
saft-io-ctl tr1 -n IO2 -c 0x0123456789abcdef 0xffffffffffffffff 1000000 0x0 0 -u




