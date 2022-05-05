#!/bin/bash

##########################
## Setup of the WR network
##########################
# TTF:
# WRS configured with dot-config_timing_mps_access
# SCU as TX node, scuxl0396
# SCU as RX node, scuxl0497

# HO:
# WRS configured with dot-config_production_mps_access_ho
# RPi with DHCP/FTP server
# Pexaria as DM, /dev/wbm1
# Pexaria as FBASTX, /dev/wbm0
# Pexp as FBASRX, /dev/wbm2

#####################################
## General setup for all timing nodes
#####################################

export FBASTX="dev/wbm0"
export FBASRX="dev/wbm2"
export addr_set_node_type="0x04060820"
export addr_get_node_type="0x04060830"
export addr_cmd="0x04060508"     # shared memory location for command buffer
export addr_cnt1="0x04060934"    # shared memory location for received frames counter
export addr_msr1="0x04060968"    # shared memory location for measurement results
export addr_eca_vld="0x04060990" # shared memory location of counter for valid actions
export addr_eca_ovf="0x04060994" # shared memory location of counter for overflow actions
export addr_senderid="0x04060998" # shared memory location of sender ID

export instr_fsm_configure=0x01 # FSM CONFIGURE state
export instr_fsm_opready=0x02   # FSM OPREADY state

export instr_set_nodetype=0x15  # set node type
export instr_set_io_oe=0x16     # set IO output enable
export instr_get_io_oe=0x17     # get IO output enable
export instr_toggle_io=0x18     # toggle IO output
export instr_load_senderid=0x19 # load sender ID

export instr_probe_sb_diob=0x20 # probe DIOB slave card on SCU bus
export instr_probe_sb_user=0x21 # probe a given slave (sys and group IDs are expected in shared mem @FBAS_SHARED_SET_SBSLAVES)

export instr_en_mps=0x30        # enable MPS signalling
export instr_dis_mps=0x31       # disable MPS signalling
export instr_st_tx_dly=0x32     # store the transmission delay measurement results to shared memory
export instr_st_ow_dly=0x33     # store the one-way delay measurement results to shared memory
export instr_st_sg_lty=0x34     # store the signalling latency measurement results to shared memory
export instr_st_ttl_ival=0x35   # store the TTL interval measurement results to shared memory

export mac_tx_node="0x00267b0004da" # sender ID of TX node
export mac_any_node="0xffffffffffff" # sender ID of any node

export evt_mps_flag_any="0xffffeeee00000000" # generator event for MPS flags
export  evt_mps_flag_ok="0xffffeeee00000001" # event to generate the MPS OK flag
export evt_mps_flag_nok="0xffffeeee00000002" # event to generate the MPS NOK flag
export evt_mps_flag_tst="0xffffeeee00000003" # event to generate the MPS TEST flag
export evt_mps_prot_std="0x1fcbfcb000000000" # event with MPS protocol (regular)
export evt_mps_prot_chg="0x1fccfcc000000000" # event with MPS protocol (change in flag)
export          evt_tlu="0xffff100000000000" # TLU event (used to catch the signal change at IO port)
export    evt_new_cycle="0xffffdddd00000000" # event for new cycle
export      evt_id_mask="0xffffffff00000000" # event mask

module_dir="${PWD/fbas*/fbas}"
fw_dir="$module_dir/fw"
fw_tx="$fw_dir/fbas.pcicontrol.bin"
fw_rx="$fw_dir/fbas.pcicontrol.bin"

user_approval() {
    echo -en "\nCONITNUE (Y/n)? "
    read -r answer

    if [ "$answer" != "y" ] && [ "$answer" != "Y" ] && [ "$answer" != "" ]; then
        exit 1
    fi
}

wait_seconds() {
    #echo "wait $1 seconds ..."
    sleep $1
}

wait_print_seconds() {
    # $1 - wait period in seconds

    if [ "$1" == "" ]; then
        return
    fi

    for i in $(seq 1 $1); do
        #echo -ne "time left (seconds): $[ $1 - $i ]\r"
        v=$[ $1 - $i ]
        v=$(printf "%*d\r" "8" $v)          # print numbers in 8 digits leading with spaces
        echo -ne "time left (seconds): $v"  # overwrite previous output
        wait_seconds 1
    done

}

check_fbastx() {
    if [ -z "$FBASTX" ]; then
        echo "FBASTX is not set"
        exit 1
    fi
}

check_fbasrx() {
    if [ -z "$FBASRX" ]; then
        echo "FBASRX is not set"
        exit 1
    fi
}

check_dev_fw() {

    echo "verify if a firmware runs on LM32"
    eb-info -w $1

    # ********************
    # * RAM @ 0x04060000 *
    # ********************
    # UserLM32
    # Stack Status:  okok
    # Project     : fbastx
    # Version     : 00.01.00
    # Platform    : pcicontrol
    # Build Date  : Fri Feb 26 12:41:13 CET 2021
    # Prepared by : ebold Enkhbold Ochirsuren <E.Ochirsuren@gsi.de>
    # Prepared on : acopc017
    # OS Version  : Linux Mint 19.1 Tessa  Linux 4.15.0-135-generic x86_64
    # GCC Version : lm32-elf-gcc(GCC)4.5.3 (build 190527-673a32-f3d6)
    # IntAdrOffs  : 0x10000000
    # SharedOffs  : 0x500
    # SharedSize  : 8K
    # FW-ID ROM will contain:

    #    09bab356 fbas: re-calculated the transmission time of a timig message via a WRS
}

list_dev_ram() {
    echo "get LM32 RAM address (host perspective)"
    eb-ls $1 | grep LM32

    # 12.14          0000000000000651:10041000           4040000  LM32-CB-Cluster
    # 12.14.5        0000000000000651:54111351           4060000  LM32-RAM-User
}

read_shared_mem() {
    # $1 - device
    # $2 - memory address

    eb-read $1 $2/4
}

write_shared_mem() {
    # $1 - device
    # $2 - memory address
    # $3 - value

    eb-write $1 $2/4 $3
}

dont_call_set_ip() {
    echo "assign static IP addresses to the timing receivers: 192.168.131.30 for wbm0, 192.168.131.40 for wbm2"
    echo "set IP address (wrc): ip set 192.168.131.30"
    echo "verify IP address (wrc): ip"
    eb-console $1
}

start_saftd() {

    check_fbastx
    check_fbasrx

    echo "terminate SAFT daemon if it's running"
    sudo killall saftd

    if [ $? -eq 0 ]; then
        echo "wait until SAFT daemon terminates"
        for i in $(seq 1 10); do
            echo -ne "time left (seconds): $[ 10 - $i ]\r"
            wait_seconds 1
        done
    fi

    echo "attach fbastx:$FBASTX and fbasrx:$FBASRX'"
    sudo saftd fbastx:$FBASTX fbasrx:$FBASRX

    #echo "attach 'wbm2' to 'fbasrx'"
    #saft-ctl fbasrx attach dev/wbm2
}

check_node() {
    # $1 - device (FBASRX or FBASTX)

    if [ "$1" != "$FBASRX" ] && [ "$1" != "$FBASTX" ]; then
        echo "unknown node type: $1"
        exit 1
    fi
}

load_fw() {
    # $1 - device (FBASRX or FBASTX)
    # $2 - firmware filename

    check_node "$1"

    unset fw_filename

    if [ "$1" == "$FBASRX" ]; then
        check_fbasrx
        fw_filename=$fw_rx
    else
        check_fbastx
        fw_filename=$fw_tx
    fi

    if [ ! -z "$2" ]; then
        fw_filename="$fw_dir/$2"
        if [ ! -f "$fw_filename" ]; then
            echo "'$fw_filename' not found. Exit"
            return 1
        fi
    fi

    echo "$1: load the LM32 firmware '$fw_filename'"
    eb-fwload $1 u 0x0 $fw_filename
    if [ $? -ne 0 ]; then
        echo "Error: failed to load LM32 FW '$fw_filename'. Exit!"
        exit 1
    fi
    wait_seconds 1

    # wrc output:
    #   fbas0: SHARED_SET_NODETYPE 0x10000694
    #   fbas0: SHARED_GET_NODETYPE 0x100006a0
    #   fbastx: CPU RAM External 0x 4060000, begin shared 0x00000500
    #
    #   pSharedCmd = 0x4060508 (CPU RAM External + begin_shared + COMMON_SHARED_CMD)
    #   pSharedSetNodeType = 0x4060694 (CPU RAM External +  begin_shared + SHARED_SET_NODETYPE - 0x10000000)
    #   pSharedGetNodeType = 0x40606a0 (CPU RAM External +  begin_shared + SHARED_GET_NODETYPE - 0x10000000)

}

configure_node() {
    # $1 - device (FBASRX or FBASTX)
    # $2 - sender node groups (SENDER_TX or SENDER_ANY or SENDER_ALL)

    check_node "$1"

    eb-write $1 $addr_cmd/4 $instr_fsm_configure
    wait_seconds 1

    # wrc output:
    #   common-fwlib: received cmd 1
    #   fbas0: received unknown command '0x00000001'
    #   common-fwlib: changed to state 3

    if [ "$1" == "$FBASRX" ]; then
        echo "set node type to FBASRX (0x1)"
        eb-write $1 $addr_set_node_type/4 0x1
        wait_seconds 1

        echo "tell LM32 to set the node type"
        eb-write $1 $addr_cmd/4 $instr_set_nodetype
        wait_seconds 1

        # wrc output:
        #   common-fwlib: common_cmdHandler received unknown command '0x00000015'
        #   fbas1: node type 1

        echo "verify the actual node type"
        eb-read $1 $addr_get_node_type/4
        wait_seconds 1

        # wrc output:
        #   00000001

        # set sender IDs
        set_senderid "$2"
    fi
}

set_senderid() {
    # $1 - sender groups, valid values: SENDER_TX, SENDER_ANY, SENDER_ALL

    # SENDER_TX - only TX node
    # SENDER_ANY - only any nodes
    # SENDER_ALL - TX and any nodes

    first_idx=1
    last_idx=15
    unset idx_mac_list  # list with idx_mac

    if [ "$1" == "SENDER_TX" ]; then
        idx_mac_list="$mac_tx_node"
    elif [ "$1" == "SENDER_ALL" ] || [ "$1" == "SENDER_ANY" ]; then
        if [ "$1" == "SENDER_ALL" ]; then
            idx_mac_list="$mac_tx_node"
            first_idx=$(( $first_idx - 1 ))
            last_idx=$(( $last_idx - 1 ))
        else
            idx_mac_list="$mac_any_node"
        fi

        # idx is used to specify an MPS channel of the same sender
        for i in $(seq $first_idx $last_idx); do
            idx=$(( $i << 48 ))
            idx_mac=$(( $idx + $mac_any_node ))
            idx_mac=$(printf "0x%x" $idx_mac)
            idx_mac_list="$idx_mac_list $idx_mac"
        done
    else
        return
    fi

    echo "tell LM32 to set the sender IDs"

    device=$FBASRX
    i=0
    for idx_mac in $idx_mac_list; do
        pos=$(( $i << 56 ))                               # position in RX buffer
        senderid=$(( $pos + $idx_mac ))                   # sender ID = position + (idx + MAC)
        senderid=$(printf "0x%x" $senderid)
        eb-write -q $device $addr_senderid/8 $senderid
        eb-read -q $device $addr_senderid/8
        eb-write $device $addr_cmd/4 $instr_load_senderid
        i=$(( $i + 1 ))
        sleep 0.2
    done

}

make_node_ready() {
    # $1 - device (FBASRX or FBASTX)

    check_node "$1"

    eb-write $1 $addr_cmd/4 $instr_fsm_opready
    wait_seconds 1

    # wrc console output:
    #   common-fwlib: received cmd 2
    #   fbas0: received unknown command '0x00000002'
    #   common-fwlib: changed to state 4
}

configure_tr() {
    # $1 - device (FBASRX or FBASTX)

    check_node "$1"

    unset tr

    if [ "$1" == "$FBASTX" ]; then
        tr="fbastx"
    else
        tr="fbasrx"
    fi

    echo "destroy all unowned ECA conditions"
    saft-ecpu-ctl $tr -x

    echo "disable all events from IO inputs to ECA"
    saft-io-ctl $tr -w

    if [ "$1" == "$FBASTX" ]; then
        echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x42"
        saft-ecpu-ctl $tr -c $evt_mps_flag_any $evt_id_mask 0 0x42 -d

        echo "configure ECA: listen for TLU event with the given ID, tag 0x43"
        saft-ecpu-ctl $tr -c $evt_tlu $evt_id_mask 0 0x43 -d

        echo "configure ECA: listen for FBAS_AUX_CYCLE event, tag 0x26"
        saft-ecpu-ctl $tr -c $evt_new_cycle $evt_id_mask 0 0x26 -d

        echo "configure TLU: on signal transition at IO2 input, it will generate a timing event with the given ID"
        saft-io-ctl $tr -n IO2 -b $evt_tlu

        echo "now events can be snooped with a following command: saft-ctl $tr -xv snoop 0 0 0"
        echo "or events can be presented by the LM32 firmware if WR console is active: $ eb-console $1"
    else
        echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x24 and 0x25"
        saft-ecpu-ctl $tr -c $evt_mps_prot_std $evt_id_mask 0 0x24 -d
        saft-ecpu-ctl $tr -c $evt_mps_prot_chg $evt_id_mask 0 0x25 -d

        echo "configure ECA: listen for FBAS_AUX_CYCLE event, tag 0x26"
        saft-ecpu-ctl $tr -c $evt_new_cycle $evt_id_mask 0 0x26 -d
    fi

    echo "show actual ECA conditions"
    saft-ecpu-ctl $tr -l

    echo "list all IO conditions in ECA"
    saft-io-ctl $tr -l
}

######################
## Make 'fbastx' ready
######################

setup_fbastx() {

    echo "set up TX node"
    load_fw "$FBASTX"

    echo "CONFIGURE state "
    configure_node "$FBASTX"

    echo "OPREADY state "
    make_node_ready "$FBASTX"

    echo "configure TR"
    configure_tr "$FBASTX"
}

####################
## Make 'fbasrx' ready
####################

setup_fbasrx() {
    # $1 - firmware filename supplied externally
    # $2 - sender node groups (SENDER_TX or SENDER_ANY or SENDER_ALL)

    echo "set up RX node"
    load_fw "$FBASRX" "$1"

    echo "CONFIGURE state "
    configure_node "$FBASRX" "$2"

    echo "OPREADY state "
    make_node_ready "$FBASRX"

    echo "configure TR"
    configure_tr "$FBASRX"
}

####################
## Reset FTRN node
#
# - FW is not re-loaded
# + reset LM32
# + set node type, if 'RX'
# + set oper. mode to 'OPREADY'
####################

reset_node() {
    # $1 - device (FBASTX or FBASRX)
    # $2 - sender node groups (SENDER_TX or SENDER_ANY or SENDER_ALL)

    check_node "$1"

    unset tr

    if [ "$1" == "$FBASRX" ]; then
        check_fbasrx
        tr="fbasrx"
    else
        check_fbastx
        tr="fbastx"
    fi

    echo "reset LM32"
    eb-reset $1 cpureset 0x0
    wait_seconds 1

    echo "CONFIGURE state "
    configure_node "$1" "$2"

    echo "OPREADY state "
    make_node_ready "$1"

    echo "show actual ECA conditions"
    saft-ecpu-ctl $tr -l

    echo "list all IO conditions in ECA"
    saft-io-ctl $tr -l
}

####################
## Test FW operation
####################

dont_call_open_wr_console() {

    check_fbastx
    check_fbasrx

    echo "open wrc for FBASTX to see the debug output"
    eb-console $FBASTX
    eb-console $FBASRX
}

##########################################################
# Pre-check for test 3:
#
# Suggest to check the IO connection between RX and TX nodes.
#
# IO connection with LEMO: RX:IO1 -> TX:IO2
##########################################################

precheck_test3() {
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

##########################################################
# Test 3:
# TX SCU sends MPS flag periodically in timing msg with event ID=0x1fcbfcb00 and
# sends MPS event immediately in timing msg with event ID=0x1fccfcc00.
#
# RX SCU drive its IO1 output according to MPS flag or on time-out.
#
# IO connection with LEMO: RX:IO1 -> TX:IO2
##########################################################

start_test3() {

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
        saft-ctl fbastx -p inject $evt_mps_flag_ok 0x0 1000000
        wait_seconds 1

        echo "idx = 0xff, flag = NOK(2) -> 1x MPS event (3x transmissions)"
        saft-ctl fbastx -p inject $evt_mps_flag_nok 0x0 1000000
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

    cnt=$(eb-read $FBASTX $addr_cnt1/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "MPS msgs (TX): $cnt ($cnt_dec)"

    cnt=$(eb-read $FBASRX $addr_eca_vld/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "MPS valid msgs (RX): $cnt ($cnt_dec)"

    cnt=$(eb-read $FBASRX $addr_eca_ovf/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "MPS overflow msgs (RX): $cnt ($cnt_dec)"

    echo -n "Transmission delay: "
    read_measurement_results $FBASTX $instr_st_tx_dly $addr_msr1 $verbosity

    echo -n "Signalling latency: "
    read_measurement_results $FBASTX $instr_st_sg_lty $addr_msr1 $verbosity

    result_ttl_ival $FBASRX $verbosity

    echo -e "\nMeasure TTL interval\n"
    measure_ttl_ival
}

measure_ttl_ival() {
    echo -n "enable MPS operation of RX: "
    enable_mps $FBASRX

    echo -e "toggle MPS operation of TX:\n"
    for i in $(seq 1 10); do
        echo -en "   $i : enable  "
        enable_mps $FBASTX
        sleep 1
        echo -en "   $i : disable "
        disable_mps $FBASTX
        sleep 1
    done

    echo -n "disable MPS operation of RX: "
    disable_mps $FBASRX

    result_ttl_ival $FBASRX "verbose"
}

disable_mps() {
    echo "Stop MPS on $1"
    eb-write $1 $addr_cmd/4 $instr_dis_mps
    wait_seconds 1
}

enable_mps() {
    echo "Start MPS on $1"
    eb-write $1 $addr_cmd/4 $instr_en_mps
    wait_seconds 1
}

disable_mps_all() {
    echo "Disable MPS"
    disable_mps $FBASTX
    disable_mps $FBASRX
}


enable_mps_all() {
    echo "Enable MPS"
    enable_mps $FBASRX
    enable_mps $FBASTX
}

##########################################################
# Test 2: Measure time between a signalling and TLU events ($evt_mps_flag_any and $evt_tlu)
#
# IO connection with LEMO: RX:IO1 -> TX:IO2
##########################################################

do_test2() {
    echo "injectg timing messages to FBASTX that simulate the FBAS class 2 signals"
    saft-ctl fbastx -p inject $evt_mps_flag_tst 0x0 1000000

    # Case 1: consider the ahead time of 500 us (flagForceLate=0)
    # wrc output (TX)
    #   fbas0: ECA action (tag 42, flag 0, ts 1604323287001000000, now 1604323287001004448, poll 4448)
    #   fbas0: ECA action (tag 43, flag 1, ts 1604323287001512575, now 1604323287011358920, poll 9846345) -> takes too long to output dbg msg!
    #
    # time between injecting MPS event and polling TLU event:
    # =   12575 ns (calculated by timestamp, 1604323287001512575 - 1604323287001000000 - 500000) -> not exact, because of ahead interval!
    #       ahead interval for sending timing messages is considered (COMMON_AHEADT = 500000 ns)

    # Case 2: ignore the ahead time of 500 us (flagForceLate=1)
    # wrc output (TX)
    #   fbas0: ECA action (tag 42, flag 0, ts 1604322044001000000, now 1604322044001004424, poll 4424)
    #   fbas0: ECA action (tag 43, flag 1, ts 1604322044001031206, now 1604322044011356712, poll 10325506) -> takes too long to output dbg msg!
    #
    # time period from detecting MPS event and detecting TLU events:
    # =   31206 ns (calculated by timestamp, 1604322044001031206 - 1604322044001000000)
    # time to transmit FBAS events (TX->RX):
    # =   26782 ns (1604322044001031206 - 1604322044001004424)

    # Case 3: ignore the ahead time of 500 us (flagForceLate=1), and output debug msg after handling the TLU events
    # wrc output (TX)
    #   fbas0: TLU evt (tag 43, flag 1, ts 1604325955001026839, now 1604325955001034872, poll 8033)
    #   fbas0: generator evt timestamps (detect 1604325955001000000, send 1604325955001004272, poll 4272)
    #
    #   fbas0: TLU evt (tag 43, flag 1, ts 1604325967001030350, now 1604325967001042848, poll 12498)      -> max poll time
    #   fbas0: generator evt timestamps (detect 1604325967001000000, send 1604325967001004384, poll 4384)
    #
    # time period from detecting MPS event and detecting TLU events:
    # =   42848 ns (calculated by timestamp, 1604325967001042848 - 1604325967001000000)
    # time to transmit FBAS events (TX->RX):
    # =   25966 ns (1604325967001030350 - 1604325967001004384)
}

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

read_measurement_results() {
    # $1 - TR device (ie., dev/wbm0)
    # $2 - instruction code to store measurement results to a location in the shared memory
    # $3 - shared memory location where measurement results are stored
    # $4 - verbosity

    device=$1
    instr_msr=$2
    addr_msr=$3

    eb-write $device $addr_cmd/4 $instr_msr

    avg=$(eb-read -q $device ${addr_msr}/8)
    avg_dec=$(printf "%d" 0x$avg)
    #echo "avg= 0x$avg (${avg_dec})"

    addr_msr=$(( $addr_msr + 8 ))
    min=$(eb-read -q $device ${addr_msr}/8)
    min_dec=$(printf "%lli" 0x$min)

    addr_msr=$(( $addr_msr + 8 ))
    max=$(eb-read -q $device ${addr_msr}/8)
    max_dec=$(printf "%d" 0x$max)

    addr_msr=$(( $addr_msr + 8 ))
    cnt_val=$(eb-read -q $device ${addr_msr}/4)
    cnt_val_dec=$(printf "%d" 0x$cnt_val)

    addr_msr=$(( $addr_msr + 4 ))
    cnt_all=$(eb-read -q $device ${addr_msr}/4)
    cnt_all_dec=$(printf "%d" 0x$cnt_all)

    if [ -n "$4" ]; then
        echo "avg=${avg_dec} min=${min_dec} max=${max_dec} cnt=${cnt_val_dec}/${cnt_all_dec}"
    else
        echo "${avg_dec} ${min_dec} ${max_dec} ${cnt_val_dec} ${cnt_all_dec}"
    fi
}

result_tx_delay() {
    # $1 - dev/wbm0
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "Transmit delay: "
    fi
    read_measurement_results $1 $instr_st_tx_dly $addr_msr1 $2
}

result_sg_latency() {
    # $1 - dev/wbm0
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "Signalling latency:   "
    fi
    read_measurement_results $1 $instr_st_sg_lty $addr_msr1 $2
}

result_ow_delay() {
    # $1 - dev/wbm0
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "One-way delay:  "
    fi
    read_measurement_results $1 $instr_st_ow_dly $addr_msr1 $2
}

result_ttl_ival() {
    # $1 - dev/wbm0
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "TTL interval:   "
    fi
    read_measurement_results $1 $instr_st_ttl_ival $addr_msr1 $2
}

read_counters() {
    # $1 - dev/wbm0

    device=$1
    addr_val="$addr_cnt1 $addr_eca_vld $addr_eca_ovf" # reg addresses as string
    set msg_cnt eca_vld eca_ovf   # labels as positional arguments ($1 $2 $3)

    printf "\n"
    for addr in $addr_val; do
        cnt=$(eb-read $device $addr/4)  # get counter value
        printf "%s @ %s: %d (0x%s)\n" $1 $addr 0x$cnt $cnt
        shift
    done
}
