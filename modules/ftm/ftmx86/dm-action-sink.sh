#!/bin/sh

# Init DM for message reception. Bind DM memory node INBOX @ "registers" to eca wb sink
# Copy data from timing message param field to Inbox on reception

DEV=dev/wbm0            # Etherbone device
TR=tr0                  # Saftlib timing receiver ID associated with DEV
DOT=show-inbox.dot      # Stub dotfile used to obtain external memory adr of INBOX node
TIC=0x14c0fc0000000000  # Timing ID condition action sink will trigger on
MSK=0xffffff0000000000  # Filter mask for Timing ID Condition
INITVAL=0x1312d00       # Initial value to be written to sink

#create dotfile
printf "digraph g {\\n name=\"Action Sink Init\";\\ngraph[]\\nINBOX [cpu=\"0\", type=\"global\", section=\"registers\", pattern=\"EXTERN\" ];\\nB0 [cpu=\"0\", type=\"block\", tperiod=\"1\", pattern=\"PAT0\"];\\nB0->INBOX [type=\"reference\", fieldtail=\"0x04\", fieldhead=\"0x00\", fieldwidth=\"32\"];\\n}" > $DOT

dm-cmd $DEV halt
sleep 0.3
dm-sched $DEV clear
dm-sched $DEV add $DOT
ADR=`dm-sched $DEV | awk '/INBOX/ {print $NF}'`
printf "ADR is $ADR. Clearing...\n"
eb-write $DEV $ADR/4 0x0
saft-wbm-ctl $TR -x; saft-wbm-ctl $TR -r 3 $ADR 0 0x5f; saft-wbm-ctl $TR -c $TIC $MSK 0 3 -d; sleep 1; saft-wbm-ctl $TR -l
printf "$TIC $INITVAL 0" > unilac_20.txt; sleep 0.1; saft-dm $TR -p -v unilac_20.txt
dm-sched $DEV clear
sleep 0.5
VAL=`eb-read $DEV $ADR/4`
printf "ADR $ADR now contains INITVAL 0x$VAL\n"
