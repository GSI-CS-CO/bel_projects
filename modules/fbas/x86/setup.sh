#!/bin/bash

##########################
## Setup of the WR network
##########################
# WRS with dot-config_integration_access_fbas (CONFIG_DOTCONF_INFO="gen_time=2021-02-11+11:42:43;gen_user=timing@tgservice;git_hash=ff32f67-dirty;role=integration_access_fbas;")
# RPi with DHCP/FTP server
# Pexaria as FBASTX, /dev/wbm0
# Pexp as FBASRX, /dev/wbm2

#####################################
## General setup for all timing nodes
#####################################

export FBASTX="dev/wbm0"
export FBASRX="dev/wbm2"
export addr_cnt1="0x04060934/4"  # shared memory location for received frames counter

function user_approval() {
    echo -en "\nCONITNUE (Y/n)? "
    read -r answer

    if [ "$answer" != "y" ] && [ "$answer" != "Y" ] && [ "$answer" != "" ]; then
        exit 1
    fi
}

function wait_seconds() {
    #echo "wait $1 seconds ..."
    sleep $1
}

function wait_print_seconds() {
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

function check_fbastx() {
    if [ -z "$FBASTX" ]; then
        echo "FBASTX is not set"
        exit 1
    fi
}

function check_fbasrx() {
    if [ -z "$FBASRX" ]; then
        echo "FBASRX is not set"
        exit 1
    fi
}

function check_dev_fw() {

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

function list_dev_ram() {
    echo "get LM32 RAM address (host perspective)"
    eb-ls $1 | grep LM32

    # 12.14          0000000000000651:10041000           4040000  LM32-CB-Cluster
    # 12.14.5        0000000000000651:54111351           4060000  LM32-RAM-User
}

function read_shared_mem() {
    # $1 - device
    # $2 - memory address

    eb-read $1 $2
}

function write_shared_mem() {
    # $1 - device
    # $2 - memory address
    # $3 - value

    eb-write $1 $2 $3
}

function dont_call_set_ip() {
    echo "assign static IP addresses to the timing receivers: 192.168.131.30 for wbm0, 192.168.131.40 for wbm2"
    echo "set IP address (wrc): ip set 192.168.131.30"
    echo "verify IP address (wrc): ip"
    eb-console $1
}

function start_saftd() {

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

######################
## Make 'fbastx' ready
######################

function setup_fbastx() {

    check_fbastx

    echo "load the LM32 firmware"
    eb-fwload $FBASTX u 0x0 $HOME/gsi_prj/bel_projects/modules/fbas/fw/fbas.bin
    wait_seconds 1

    # wrc output:
    #   fbas0: SHARED_SET_NODETYPE 0x10000694
    #   fbas0: SHARED_GET_NODETYPE 0x100006a0
    #   fbastx: CPU RAM External 0x 4060000, begin shared 0x00000500
    #
    #   pSharedCmd = 0x4060508 (CPU RAM External + begin_shared + COMMON_SHARED_CMD)
    #   pSharedSetNodeType = 0x4060694 (CPU RAM External +  begin_shared + SHARED_SET_NODETYPE - 0x10000000)
    #   pSharedGetNodeType = 0x40606a0 (CPU RAM External +  begin_shared + SHARED_GET_NODETYPE - 0x10000000)

    echo "CONFIGURE state "
    eb-write $FBASTX 0x4060508/4 0x1
    wait_seconds 1

    # wrc output:
    #   common-fwlib: received cmd 1
    #   fbas0: received unknown command '0x00000001'
    #   common-fwlib: changed to state 3

    echo "OPREADY state "
    eb-write $FBASTX 0x4060508/4 0x2
    wait_seconds 1

    # wrc console output:
    #   common-fwlib: received cmd 2
    #   fbas0: received unknown command '0x00000002'
    #   common-fwlib: changed to state 4
    #   fbas0: now 1604321478293723256, elap 47781578728

    echo "destroy all unowned ECA conditions"
    saft-ecpu-ctl fbastx -x

    echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x42"
    saft-ecpu-ctl fbastx -c 0xffffeeee00000000 0xffffffff00000000 0 0x42 -d

    echo "configure ECA: listen for TLU event with the given ID, tag 0x43"
    saft-ecpu-ctl fbastx -c 0xffff100000000000 0xffffffff00000000 0 0x43 -d

    echo "configure ECA: listen for FBAS_AUX_CYCLE event, tag 0x26"
    saft-ecpu-ctl fbastx -c 0xffffdddd00000000 0xffffffff00000000 0 0x26 -d

    echo "show actual ECA conditions"
    saft-ecpu-ctl fbastx -l

    echo "disable all events from IO inputs to ECA"
    saft-io-ctl fbastx -w

    echo "configure TLU: on signal transition at IO2 input, it will generate a timing event with the given ID"
    saft-io-ctl fbastx -n IO2 -b 0xffff100000000000

    echo "now both events can be snooped with a following command: saft-ctl fbastx -xv snoop 0 0 0"
    echo "or both events can be presented by the LM32 firmware if WR console is active: $ eb-console $FBASTX"

    echo "list all IO conditions in ECA"
    saft-io-ctl fbastx -l
}

####################
## Make fbasrx ready
####################

function setup_fbasrx() {

    check_fbasrx

    echo "load the LM32 firmware"
    eb-fwload $FBASRX u 0x0 $HOME/gsi_prj/bel_projects/modules/fbas/fw/fbas.bin
    wait_seconds 1

    # wrc output:
    #   fbas0: SHARED_SET_NODETYPE 0x10000694
    #   fbas0: SHARED_GET_NODETYPE 0x100006a0
    #   fbas0: CPU RAM External 0x 4060000, begin shared 0x00000500
    #   fbas0: COMMON_SHARED_BEGIN 0x10000500
    #
    #   pSharedCmd = 0x4060508 (CPU RAM External + begin_shared + COMMON_SHARED_CMD)
    #   pSharedSetNodeType = 0x4060694 (CPU RAM External +  begin_shared + SHARED_SET_NODETYPE - 0x10000000)

    echo "CONFIGURE state "
    eb-write $FBASRX 0x4060508/4 0x1
    wait_seconds 1

    echo "set node type to FBASRX (0x1)"
    echo "write node type (0x1) to dedicated memory location"
    eb-write $FBASRX 0x4060820/4 0x1
    wait_seconds 1

    echo "tell LM32 to set the node type"
    eb-write $FBASRX 0x4060508/4 0x15
    wait_seconds 1

    # wrc output:
    #   common-fwlib: common_cmdHandler received unknown command '0x00000015'
    #   fbas1: node type 1

    echo "verify the actual node type"
    eb-read $FBASRX 0x4060830/4
    wait_seconds 1

    # wrc output:
    #   00000001

    echo "OPREADY state "
    eb-write $FBASRX 0x4060508/4 0x2

    # wrc output:
    #   common-fwlib: changed to state 4
    #   fbas1: now 1604322219371061616, elap 479662071872

    echo "destroy all unowned ECA conditions"
    saft-ecpu-ctl fbasrx -x

    echo "configure ECA: set FBAS_IO_ACTION for LM32 channel, tag 0x24 and 0x25"
    saft-ecpu-ctl fbasrx -c 0x1fcbfcb000000000 0xffffffff00000000 0 0x24 -d
    saft-ecpu-ctl fbasrx -c 0x1fccfcc000000000 0xffffffff00000000 0 0x25 -d

    echo "configure ECA: listen for FBAS_AUX_CYCLE event, tag 0x26"
    saft-ecpu-ctl fbasrx -c 0xffffdddd00000000 0xffffffff00000000 0 0x26 -d

    echo "show actual ECA conditions"
    saft-ecpu-ctl fbasrx -l

    echo "disable all events from IO inputs to ECA"
    saft-io-ctl fbasrx -w

    echo "list all IO conditions in ECA"
    saft-io-ctl fbastx -l
}

####################
## Test FW operation
####################

function dont_call_open_wr_console() {

    check_fbastx
    check_fbasrx

    echo "open wrc for FBASTX to see the debug output"
    eb-console $FBASTX
    eb-console $FBASRX
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

function do_test3() {
    echo "Step 1: test TLU action for TX"
    echo "Snoop TLU event (for IO action) on 1st terminal invoke command given below:"
    echo "saft-ctl fbastx -xv snoop 0xffff100000000000 0xffffffff00000000 0"

    user_approval

    echo "Drive IO1 of RX on 2nd terminal:"
    echo "saft-io-ctl fbasrx -n IO1 -d 1"
    echo "saft-io-ctl fbasrx -n IO1 -d 0"

    user_approval

    echo "if on 1st terminal some events like 'tDeadline: 2020-11-02 17:24:49.591537414 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001 Param: 0x0000000000000000!late (by 8186 ns)' is displayed, then it's ready for next step"

    echo "Step 2: enable MPS task on RX and TX nodes"
    echo "for TX: enable sending MPS flags and events, you will see EB frames in wireshark, verify their event ID, MPS flag etc"
    echo "for RX: enable monitoring lifetime of received MPS flags"
    enable_mps_all

    echo "Step 3: inject timing events locally to generate MPS event"
    echo "OK(1) flag, grpID=1, evtID=0  -> 1x MPS event (1x transmission)"
    saft-ctl fbastx -p inject 0xffffeeee01010000 0x0 1000000
    wait_seconds 1

    echo "NOK(2) flag, grpID=1, evtID=0 -> 1x MPS event (3x transmissions)"
    saft-ctl fbastx -p inject 0xffffeeee02010000 0x0 1000000
    wait_seconds 1

    echo "send OK and NOK each 5x times -> 10x MPS events (20x transmissions)"
    for i in seq 1 5; do
        saft-ctl fbastx -p inject 0xffffeeee01010000 0x0 1000000
        wait_seconds 1

        saft-ctl fbastx -p inject 0xffffeeee02010000 0x0 1000000
        wait_seconds 1
    done

    echo "If you see 12x IO events in the snooper output, then basically all works."

    # snooper output (saft-ctl fbastx -xv snoop 0xffff100000000000 0xffffffff00000000 0)
    #   tDeadline: 2020-11-02 13:55:53.001000000 FID: 0xf GID: 0x0fff EVTNO: 0x0eee Other: 0xe01010000 Param: 0x0000000000000000
    #   tDeadline: 2020-11-02 13:55:53.001026455 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001 Param: 0x0000000000000000!late (by 326545 ns)
    #   tDeadline: 2020-11-02 13:55:58.001000000 FID: 0xf GID: 0x0fff EVTNO: 0x0eee Other: 0xe02010000 Param: 0x0000000000000000
    #   tDeadline: 2020-11-02 13:55:58.001026654 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000000 Param: 0x0000000000000000!late (by 292578 ns)
    #   tDeadline: 2020-11-02 13:56:04.001000000 FID: 0xf GID: 0x0fff EVTNO: 0x0eee Other: 0xe01010000 Param: 0x0000000000000000
    #   tDeadline: 2020-11-02 13:56:04.001030279 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001 Param: 0x0000000000000000!late (by 300977 ns)
    #   tDeadline: 2020-11-02 13:56:08.001000000 FID: 0xf GID: 0x0fff EVTNO: 0x0eee Other: 0xe02010000 Param: 0x0000000000000000
    #   tDeadline: 2020-11-02 13:56:08.001026822 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000000 Param: 0x0000000000000000!late (by 292162 ns)
    #   tDeadline: 2020-11-02 13:56:12.001000000 FID: 0xf GID: 0x0fff EVTNO: 0x0eee Other: 0xe01010000 Param: 0x0000000000000000
    #   tDeadline: 2020-11-02 13:56:12.001026879 FID: 0xf GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001 Param: 0x0000000000000000!late (by 289745 ns)

    # wrc output (TX) - debug output msg that shows transmission delay and duration for forwarding MPS events
    #   fbas0: enabled MPS 70000000
    #   fbas0: dly=22535, fwd=34720
    #   fbas0: dly=22678, fwd=41768
    #   fbas0: dly=26239, fwd=38552
    #   fbas0: dly=22518, fwd=42128
    #   fbas0: dly=22695, fwd=35096

    echo "Send 'new cycle' in 5 seconds ..."
    wait_seconds 5
    saft-ctl fbastx -p inject 0xffffdddd00000000 0x0 1000000
    saft-ctl fbasrx -p inject 0xffffdddd00000000 0x0 1000000

    echo "Disable MPS task on TX and RX nodes"
    disable_mps_all

    echo "Print counter value of TX"
    eb-read $FBASTX $addr_cnt1

    echo "Print counter value of RX"
    eb-read $FBASRX $addr_cnt1
}

function disable_mps() {
    echo "Stop MPS on $1"
    eb-write $1 0x4060508/4 0x31
    wait_seconds 1
}

function enable_mps() {
    echo "Start MPS on $1"
    eb-write $1 0x4060508/4 0x30
    wait_seconds 1
}

function disable_mps_all() {
    echo "Disable MPS"
    disable_mps $FBASTX
    disable_mps $FBASRX
}


function enable_mps_all() {
    echo "Enable MPS"
    enable_mps $FBASRX
    enable_mps $FBASTX
}

##########################################################
# Test 2: Measure time between a signalling and TLU events (0xffffeeee00000000 and 0xffff100000000000)
#
# IO connection with LEMO: RX:IO1 -> TX:IO2
##########################################################

function do_test2() {
    echo "injectg timing messages to FBASTX that simulate the FBAS class 2 signals"
    saft-ctl fbastx -p inject 0xffffeeee00000000 0x0 1000000

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

function report_two_senders_result() {
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

    echo "Result of two_senders test: $result"
    echo "Received: $rx_count of $tx_count"
}
