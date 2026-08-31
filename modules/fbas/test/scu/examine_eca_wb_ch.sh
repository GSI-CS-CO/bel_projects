#!/bin/sh

# A simple shell script to examine the operaiton of the ECA WB channel.
# This script should run locally on SCU and requires some saft tools and
# an user file with test schedule.

# In this example the ECA WB channel is used to record the specified event
# into the specified shared memory location.

# The examination is done in following steps:
# - clear any conditions and macros that trigger the ECA WB channel
# - create conditions for the specified event (identified by EVT_ID)
# - create macros that record parameter and timestamp of the target event to the specified shared memory (BUF_BASE)
# - inject the test schedule (with verbose output)
# - and show the recorded value

# defines
TR=tr0                       # timing receiver
DEV="dev/wbm0"               # timing receiver
EVT_ID=0x1fcbfcb000000000    # base event ID
EVT_MASK=0xffffffffffffffff  # event mask
BUF_BASE=0x20180000          # buffer base address (used to store the values)
N_IDX=4                      # number of macro indices (for each event)
N_EMR=16                     # number of emitters
EVT_FILENAME="simple_mps_events.sched" # file name (contains the test schedule with the base event)

# clear all existing conditions and macros
saft-wbm-ctl $TR -x

# create event conditions
for i in $(seq 0 $(($N_IDX - 1))); do
    saft-wbm-ctl $TR -c $EVT_ID $EVT_MASK 0 $i -d
done

# create macros
saft-wbm-ctl $TR -r 1 $(($BUF_BASE +  0)) 0 0x4f     # high-bits of the parameter field
saft-wbm-ctl $TR -r 2 $(($BUF_BASE +  4)) 0 0x5f     # low-bits of the parameter field
saft-wbm-ctl $TR -r 3 $(($BUF_BASE +  8)) 0 0x8f     # high-bits of the timestamp field
saft-wbm-ctl $TR -r 4 $(($BUF_BASE + 12)) 0 0x9f     # low-bits of the timestamp field

# inject events locally
saft-dm $TR $EVT_FILENAME -v

# read back the recorded values
for i in $(seq 0 $(($N_IDX - 1))); do
    offset=$(( $i * 4))
    eb-read $DEV $(($BUF_BASE + $offset))/4
done
