#!/bin/bash

##########################
## Setup of the WR network
##########################
# TTF:
# WRS configured with dot-config_timing_access_fbas
# SCU as TX node, scuxl0396
# SCU as RX node, scuxl0497

# HO:
# WRS configured with dot-config_home_access_fbas
# RPi with DHCP/FTP server
# Pexaria as DM, /dev/wbm1
# Pexaria as FBASTX, /dev/wbm0
# Pexp as FBASRX, /dev/wbm2

source ${PWD/fbas*/fbas}/test/scu/setup_local.sh  # source shell script

##########################################################
# Wrapper functions
##########################################################

setup_fbastx() {

    setup_mpstx
}

setup_fbasrx() {
    # $1 - firmware filename supplied externally
    # $2 - sender node groups (SENDER_TX or SENDER_ANY or SENDER_ALL)
    # $[3:] - sender ID(s) of SENDER_TX

    setup_mpsrx "$@"
}

##########################################################
# Basic test:
# TX SCU sends MPS flag periodically in timing msg with event ID=0x1fcbfcb0 and
# sends MPS event immediately in timing msg with event ID=0x1fccfcc0.
#
# RX SCU drive its IO1 output according to MPS flag or on time-out.
#
# IO connection with LEMO: RX:IO1 -> TX:IO2
##########################################################

precheck_basic() {
    echo "Step 1: test TLU action for TX"
    echo "Snoop TLU event (for IO action) on 1st terminal invoke command given below:"
    echo "saft-ctl fbastx -xv snoop $evt_tlu $evt_id_mask 0"

    user_approval

    echo "Drive IO1 of RX on 2nd terminal:"
    echo "saft-io-ctl fbasrx -n IO1 -o 1 -d 1"
    echo "saft-io-ctl fbasrx -n IO1 -o 1 -d 0"

    user_approval

    echo "if on 1st terminal some events like 'tDeadline: 2020-11-02 17:24:49.591537414 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001 Param: 0x0000000000000000!late (by 8186 ns)' is displayed, then it's ready for next step"
}

start_basic() {

    verbosity="verbose"

    echo "Step 2: enable MPS task on RX and TX nodes"
    echo "for TX: enable sending MPS flags and events, you will see EB frames in wireshark, verify their event ID, MPS flag etc"
    echo "for RX: enable monitoring lifetime of received MPS flags"
    enable_mps_all

    echo "Step 3: inject timing events locally to generate MPS event"

    total=5
    echo "send OK and NOK each $total times -> $(( $total * 2)) MPS events ($(( $total * 4 )) transmissions)"
    for i in $(seq 1 $total); do
        echo "idx = 0xff, flag = OK(1)  -> 1x MPS event (1x transmission)"
        saft-ctl fbastx -p inject $evt_mps_1_ok 0x0 1000000
        wait_seconds 1

        echo "idx = 0xff, flag = NOK(2) -> 1x MPS event (3x transmissions)"
        saft-ctl fbastx -p inject $evt_mps_1_nok 0x0 1000000
        wait_seconds 1
    done

    echo "If you see $(( $total * 2)) IO events in the snooper output, then basically all works."

    # snooper output (saft-ctl fbastx -xv snoop $evt_tlu $evt_id_mask 0)
    #   tDeadline: 2022-04-27 08:19:54.001037375 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001 Param: 0x0000000000000000!late (by 4089 ns)
    #   tDeadline: 2022-04-27 08:19:55.001036422 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000000 Param: 0x0000000000000000!late (by 8186 ns)

    # wrc output (eb-console dev/wbm2) - debug output msg that shows transmission delay and duration for forwarding MPS events
    #   txd @0x10000968 avg=35191 min=31897 max=43013 cnt=10/10
    #   sgl @0x10000968 avg=48712 min=45112 max=56288 cnt=10/10

    pause=2
    echo "Send 'new cycle' in $pause seconds ..."
    wait_seconds $pause
    saft-ctl fbasrx -p inject $evt_new_cycle 0x0 1000000

    echo "Disable MPS task on TX and RX nodes"
    disable_mps_all

    cnt=$(eb-read $tx_node_dev $addr_cnt/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "MPS msgs (TX): $cnt ($cnt_dec)"

    cnt=$(eb-read $rx_node_dev $addr_eca_vld/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "MPS valid msgs (RX): $cnt ($cnt_dec)"

    cnt=$(eb-read $rx_node_dev $addr_eca_ovf/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "MPS overflow msgs (RX): $cnt ($cnt_dec)"

    echo -n "MPS event handling: "
    read_measurement_results $tx_node_dev $instr_st_eca_handle $addr_avg $verbosity

    echo -n "Transmission delay: "
    read_measurement_results $tx_node_dev $instr_st_tx_dly $addr_avg $verbosity

    echo -n "Signalling latency: "
    read_measurement_results $tx_node_dev $instr_st_sg_lty $addr_avg $verbosity

    result_ttl_ival $rx_node_dev $verbosity

    echo -e "\nMeasure TTL interval\n"
    measure_ttl_ival
}

measure_ttl_ival() {
    echo -n "enable MPS operation of RX: "
    enable_mps $rx_node_dev

    echo -e "toggle MPS operation of TX:\n"
    for i in $(seq 1 10); do
        echo -en "   $i : enable  "
        enable_mps $tx_node_dev
        sleep 1
        echo -en "   $i : disable "
        disable_mps $tx_node_dev
        sleep 1
    done

    echo -n "disable MPS operation of RX: "
    disable_mps $rx_node_dev

    result_ttl_ival $rx_node_dev "verbose"
}

##########################################################
# Test with 2 senders and 1 receiver:
#  senders: datamaster and tx_node_dev node
#  note: LEMO cable connection between tx_node_dev and rx_node_dev nodes is not required
##########################################################

report_two_senders_result() {
    # $1 - RX count (in hex format without 0x, like 0000abcd)
    # $2 - TX count by sender node
    # $3 - TX count by datamaster

    rx_count=$((16#$1))
    tx_node=$((16#$2))
    tx_dm=$((16#$3))
    tx_count=$(( $tx_node + $tx_dm ))

    echo "Received:           $1 ($rx_count)"
    echo "Sent by TX node:    $2 ($tx_node)"
    echo "Sent by Datamaster: $3 ($tx_dm)"

    result="PASSED"

    if [ $rx_count -ne $tx_count ]; then
        result="FAILED"
    fi

    echo "Test result: --- $result ---"
    echo "Received: $rx_count of $tx_count"
}
