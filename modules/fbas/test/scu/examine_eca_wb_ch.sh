#!/bin/sh

# A simple shell script to examine the operaiton of the ECA WB channel.
# This script should run locally on SCU and requires some saft tools and
# an user file with test schedule. A target SCU must have the DIOB card.

# In the 1st part the ECA WB channel is used to transfer parameter and
# timestamp of the selected event into the specified shared memory location.

# In the 2nd part the ECA WB channel is used to transfer the 16 or 8 bits of
# parameter of the selected events into a SCU bus slave (eg., echo register
# of the DIOB card installed in the slot 2 of scuxl0411).

# The examination is done in following steps:
# - clear any conditions and macros that trigger the ECA WB channel
# - create conditions for the specified event(s)
# - create macros that transfer the parameter and timestamp of the specified event to target locations
# - inject the test schedule (with verbose output)
# - and show/verify the transferred value(s)

# ECA to shared memory transfer is simple.

# But, ECA to SCU bus slave transfer is special: 32-bit to 16-bit bus transfer with
# data width of 8 or 16 bits. Moreover, 32-bit data is part of an initial 64-bit data
# (parameter in timing message).
# This allow only following transfer to the target address in SCU bus slave:
# - high 16 bits of low/upper 32 bits in parameter to the target address
# - but, low 16 bits of low/upper 32 bits in parameter to the next higher address of the target address
# Hence, only high 16 bits of each 32-bit part of parameter can be written to the echo register.

# To verify the ECA to SCU bus slave transfer (e.g., echo register) externally:
# eb-write dev/wbm0 0x440020/2 0xcafe
# eb-read dev/wbm0 0x440020/2          # should return 'cafe'
# eb-write dev/wbm0 0x440022/2 0xcafe  # not allowed, read-only status register
# etherbone cycle error: one or more operations failed

# defines
TR=tr0                       # timing receiver
DEV="dev/wbm0"               # timing receiver
N_EVT=9                      # total number of events (ECA-to-WBM)
EVT_ID_0=0x1fcbfcb000000000  # event ID
EVT_ID_1=0x1fcbfcb000000001  # event ID
EVT_ID_2=0x1fcbfcb000000002  # event ID
EVT_ID_3=0x1fcbfcb000000003  # event ID
EVT_ID_4=0x1fcbfcb000000004  # event ID
EVT_ID_5=0x1fcbfcb000000005  # event ID
EVT_ID_6=0x1fcbfcb000000006  # event ID
EVT_ID_7=0x1fcbfcb000000007  # event ID
EVT_ID_8=0x1fcbfcb000000008  # event ID
EVT_MASK=0xffffffffffffffff  # event mask
EVT_PARAM=0x1122334455667788 # event parameter
BUF_BASE=0x20180000          # buffer base address (used to store the values)
ECHO_REG=0x440020            # 0x440020 for EtherBone master, echo register of the SCU bus slave, slot 2 (scuxl0411)
STATUS_REG=0x440022          # status register of the SCU bus slave, slot 2 (scuxl0411)
ECHO_INIT_VAL=0xfeed         # initial value for the echo register
N_IDX=4                      # number of macro indices (for each event)
N_EMR=16                     # number of emitters
EVT_FILENAME="simple_mps_events.sched" # file name (contains the test schedule with the base event)

reset_echo_reg() {
    # $1 - 16-bit hex value
    eb-write $DEV $ECHO_REG/2 $1
}

# 1. ECA to shared memory
echo "### Examine the ECA to LM32 shared memory transfer ###"

# clear all existing conditions and macros
saft-wbm-ctl $TR -x

# create event conditions
for i in $(seq 1 $N_IDX); do
    saft-wbm-ctl $TR -c $EVT_ID_0 $EVT_MASK 0 $i -d
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

# 2. ECA to SCU bus
echo "### Examine the ECA to SCU bus transfer ###"

echo "- clear all existing conditions and macros"
saft-wbm-ctl $TR -x

echo "- create ECA conditions for following events: $EVT_ID_0 .. $EVT_ID_8"

for i in $(seq 0 $(($N_EVT - 1))); do
    evt_id=$(( EVT_ID_0 + i ))
    saft-wbm-ctl $TR -c $evt_id $EVT_MASK 0 $((i + 1)) -d
done

echo "- create macros"

for i in $(seq 0 $(($N_EVT - 1))); do
    case $i in
        0)
            flag=0x5c
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c9-12)"
            echo "- on event=$EVT_ID_0 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write high 16 bits of low 32 bits in parameter to the echo register)"
            ;;
        1)
            flag=0x58
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c9-10).."
            echo "- on event=$EVT_ID_1 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write high byte of high 16 bits of low 32 bits in parameter to the echo register)"
            ;;
        2)
            flag=0x54
            val16="${EVT_PARAM##0x}"
            val16="..$(echo $val16 | cut -c11-12)"
            echo "- on event=$EVT_ID_2 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write low byte of high 16 bits of low 32 bits in parameter to the echo register)"
            ;;
        3)
            flag=0x53
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c13-16)"
            echo "- on event=$EVT_ID_3 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write low 16 bits of low 32 bits in parameter to the next 16-bit address of the echo register -> it will fail, cannot write to the status register)"
            ;;
        4)
            flag=0x52
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c13-14).."
            echo "- on event=$EVT_ID_4 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write high byte of low 16 bits of low 32 bits in parameter to the next 16-bit address of the echo register -> it will fail, cannot write to the status register)"
            ;;
        5)
            flag=0x51
            val16="${EVT_PARAM##0x}"
            val16="..$(echo $val16 | cut -c15-16)"
            echo "- on event=$EVT_ID_5 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write low byte of low 16 bits of low 32 bits in parameter to the next 16-bit address of the echo register -> it will fail, cannot write to the status register)"
            ;;
        6)
            flag=0x4c
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c1-4)"
            echo "- on event=$EVT_ID_6 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write high 16 bits of upper 32 bits in parameter to the echo register)"
            ;;
        7)
            flag=0x48
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c1-2).."
            echo "- on event=$EVT_ID_7 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write high byte of high 16 bits of upper 32 bits in parameter to the echo register)"
            ;;
        8)
            flag=0x44
            val16="${EVT_PARAM##0x}"
            val16="..$(echo $val16 | cut -c3-4)"
            echo "- on event=$EVT_ID_8 parameter=$EVT_PARAM: $val16 -> $ECHO_REG (write low byte of high 16 bits of upper 32 bits in parameter to the echo register)"
            ;;
        *)
            echo "Event $i is out of limit $N_EVT. Exit!" && exit 1
            ;;
    esac

    idx=$((i + 1))
    saft-wbm-ctl $TR -r $idx $ECHO_REG 0 $flag
done

# inject events and verify the ECA to SCU bus transfer

passed=0
for i in $(seq 0 $(($N_EVT - 1))); do
    echo
    echo "- set the echo register at SCU bus address $ECHO_REG=$ECHO_INIT_VAL"
    reset_echo_reg $ECHO_INIT_VAL
    evt_id=$(( EVT_ID_0 + i ))
    echo "- inject $(printf 0x%x $evt_id) with parameter $EVT_PARAM"
    saft-ctl $TR inject $evt_id $EVT_PARAM 0

    case $i in
        0)
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c9-12)"
            echo "- read the value from $ECHO_REG ($val16 is expected)"
            reg_val=$(eb-read $DEV $ECHO_REG/2)
            if [ "$reg_val" != "$val16" ]; then
                echo "- FAIL: $reg_val != $val16"
            else
                echo "+ PASS: $reg_val == $val16"
                passed=$((passed + 1))
            fi
            ;;
        1)
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c9-10).."
            echo "- read the value from $ECHO_REG ($val16 is expected)"
            reg_val=$(eb-read $DEV $ECHO_REG/2)
            reg_val_hi="$(echo $reg_val | cut -c1-2)"
            val16_hi="$(echo $val16 | cut -c1-2)"
            if [ "$reg_val_hi" != "$val16_hi" ]; then
                echo "- FAIL: ${reg_val_hi}.. (of $reg_val) != $val16"
            else
                echo "+ PASS: ${reg_val_hi}.. (of $reg_val) == $val16"
                passed=$((passed + 1))
            fi
            ;;
        2)
            val16="${EVT_PARAM##0x}"
            val16="..$(echo $val16 | cut -c11-12)"
            echo "- read the value from $ECHO_REG ($val16 is expected)"
            reg_val=$(eb-read $DEV $ECHO_REG/2)
            reg_val_lo="$(echo $reg_val | cut -c3-4)"
            val16_lo="$(echo $val16 | cut -c3-4)"
            if [ "$reg_val_lo" != "$val16_lo" ]; then
                echo "- FAIL: ..${reg_val_lo} (of $reg_val) != $val16"
            else
                echo "+ PASS: ..${reg_val_lo} (of $reg_val) == $val16"
                passed=$((passed + 1))
            fi
            ;;
        3)
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c13-16)"
            echo "- read the value from $STATUS_REG ($val16 is NOT expected)"
            reg_val=$(eb-read $DEV $STATUS_REG/2)
            if [ "$reg_val" = "$val16" ]; then
                echo "- FAIL: $reg_val == $val16"
            else
                echo "+ PASS: $reg_val != $val16"
                passed=$((passed + 1))
            fi
            ;;
        4)
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c13-14).."
            echo "- read the value from $STATUS_REG ($val16 is NOT expected)"
            reg_val=$(eb-read $DEV $STATUS_REG/2)
            reg_val_hi="$(echo $reg_val | cut -c1-2)"
            val16_hi="$(echo $val16 | cut -c1-2)"
            if [ "$reg_val_hi" = "$val16_hi" ]; then
                echo "- FAIL: ${reg_val_hi}.. (of $reg_val) == $val16"
            else
                echo "+ PASS: ${reg_val_hi}.. (of $reg_val) != $val16"
                passed=$((passed + 1))
            fi
            ;;
        5)
            val16="${EVT_PARAM##0x}"
            val16="..$(echo $val16 | cut -c15-16)"
            echo "- read the value from $STATUS_REG ($val16 is NOT expected)"
            reg_val=$(eb-read $DEV $STATUS_REG/2)
            reg_val_lo="$(echo $reg_val | cut -c3-4)"
            val16_lo="$(echo $val16 | cut -c3-4)"
            if [ "$reg_val_lo" = "$val16_lo" ]; then
                echo "- FAIL: ..${reg_val_lo} (of $reg_val) == $val16"
            else
                echo "+ PASS: ..${reg_val_lo} (of $reg_val) != $val16"
                passed=$((passed + 1))
            fi
            ;;
        6)
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c1-4)"
            echo "- read the value from $ECHO_REG ($val16 is expected)"
            reg_val=$(eb-read $DEV $ECHO_REG/2)
            if [ "$reg_val" = "$val16" ]; then
                echo "+ PASS: $reg_val == $val16"
                passed=$((passed + 1))
            else
                echo "- FAIL: $reg_val != $val16"
            fi
            ;;
        7)
            val16="${EVT_PARAM##0x}"
            val16="$(echo $val16 | cut -c1-2).."
            echo "- read the value from $ECHO_REG ($val16 is expected)"
            reg_val=$(eb-read $DEV $ECHO_REG/2)
            reg_val_hi="$(echo $reg_val | cut -c1-2)"
            val16_hi="$(echo $val16 | cut -c1-2)"
            if [ "$reg_val_hi" = "$val16_hi" ]; then
                echo "+ PASS: ${reg_val_hi}.. (of $reg_val) == $val16"
                passed=$((passed + 1))
            else
                echo "- FAIL: ${reg_val_hi}.. (of $reg_val) != $val16"
            fi
            ;;
        8)
            val16="${EVT_PARAM##0x}"
            val16="..$(echo $val16 | cut -c3-4)"
            echo "- read the value from $ECHO_REG ($val16 is expected)"
            reg_val=$(eb-read $DEV $ECHO_REG/2)
            reg_val_lo="$(echo $reg_val | cut -c3-4)"
            val16_lo="$(echo $val16 | cut -c3-4)"
            if [ "$reg_val_lo" = "$val16_lo" ]; then
                echo "+ PASS: ..${reg_val_lo} (of $reg_val) == $val16"
                passed=$((passed + 1))
            else
                echo "- FAIL: ..${reg_val_lo} (of $reg_val) != $val16"
            fi
            ;;
        *)
            echo "Event $i is out of limit $N_EVT. Exit!" && exit 1
            ;;
    esac

done

echo
echo "- passes: $passed of $N_EVT"
exit $(($N_EVT - $passed))
