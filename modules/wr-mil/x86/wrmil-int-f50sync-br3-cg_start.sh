#!/bin/sh
# startup script for WRF50 50 Hz generation (lab)
#
#set -x

logger 'starting 50 Hz clock'
# generate 100 kHz clock with 20ns phase offset
saft-io-ctl  tr1 -n IO3 -o 1 -t 0
saft-clk-gen tr1 -n IO3 -f 50 20


