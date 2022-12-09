#!/bin/sh
# script for deployment on ASL
. /etc/functions

log 'initializing clockgen'

ARCH=$(/bin/uname -m)
HOSTNAME=$(/bin/hostname -s)

log 'starting 100kHz clock'
# generate 100 kHz clock with 20ns phase offset
saft-io-ctl  tr1 -n IO3 -o 1 -t 0
saft-clk-gen tr1 -n IO3 -f 100000 20

log 'starting 25MHz clock'
# generate 25 MHz clock with 0ns phase offset
saft-io-ctl  tr1 -n IO2 -o 1 -t 0
saft-clk-gen tr1 -n IO2 -f 25000000 0

log 'clocks started'
