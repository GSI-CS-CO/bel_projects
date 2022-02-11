#!/bin/sh

# Set up MPS nodes (run it locally on SCU)

export DEV_TX="dev/wbm0"
export DEV_RX="dev/wbm0"
export addr_cmd="0x20140508/4"   # shared memory location for command buffer
export addr_cnt1="0x20140934/4"  # shared memory location for received frames counter
export addr_msr1="0x20140968"    # shared memory location for measurement results
export FW_TX="fbas.scucontrol.bin"
export FW_RX="fbas.scucontrol.bin"

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

check_mpstx() {
    if [ -z "$DEV_TX" ]; then
        echo "DEV_TX is not set"
        exit 1
    fi
}

check_mpsrx() {
    if [ -z "$DEV_RX" ]; then
        echo "DEV_RX is not set"
        exit 1
    fi
}

check_dev_fw() {

    echo "verify if a firmware runs on LM32"
    eb-info -w $1
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

    eb-read $1 $2
}

write_shared_mem() {
    # $1 - device
    # $2 - memory address
    # $3 - value

    eb-write $1 $2 $3
}

start_saftd() {

    check_mpstx
    check_mpsrx

    echo "terminate SAFT daemon if it's running"
    sudo killall saftd

    if [ $? -eq 0 ]; then
        echo "wait until SAFT daemon terminates"
        for i in $(seq 1 10); do
            echo -ne "time left (seconds): $[ 10 - $i ]\r"
            wait_seconds 1
        done
    fi

    echo "attach mpstx:$DEV_TX and mpsrx:$DEV_RX'"
    sudo saftd mpstx:$DEV_TX mpsrx:$DEV_RX
}

######################
## Make 'mpstx' ready
######################

setup_mpstx() {

    check_mpstx

    echo "load the LM32 firmware"
    eb-fwload $DEV_TX u 0x0 $FW_TX
    wait_seconds 1

    echo "CONFIGURE state "
    eb-write $DEV_TX $addr_cmd 0x1
    wait_seconds 1

    echo "OPREADY state "
    eb-write $DEV_TX $addr_cmd 0x2
    wait_seconds 1

    echo "destroy all unowned ECA conditions"
    saft-ecpu-ctl tr0 -x

    echo "configure ECA: set FBAS_GEN_EVT for LM32 channel, tag 0x42"
    saft-ecpu-ctl tr0 -c 0xffffeeee00000000 0xffffffff00000000 0 0x42 -d

    echo "configure ECA: listen for TLU event with the given ID, tag 0x43"
    saft-ecpu-ctl tr0 -c 0xffff100000000000 0xffffffff00000000 0 0x43 -d

    echo "configure ECA: listen for FBAS_AUX_CYCLE event, tag 0x26"
    saft-ecpu-ctl tr0 -c 0xffffdddd00000000 0xffffffff00000000 0 0x26 -d

    echo "show actual ECA conditions"
    saft-ecpu-ctl tr0 -l

    echo "disable all events from IO inputs to ECA"
    saft-io-ctl tr0 -w

    echo "configure TLU: on signal transition at B2 input, it will generate a timing event with the given ID"
    saft-io-ctl tr0 -n B2 -b 0xffff100000000000

    echo "now both events can be snooped with a following command: saft-ctl tr0 -xv snoop 0 0 0"
    echo "or both events can be presented by the LM32 firmware if WR console is active: $ eb-console $DEV_TX"

    echo "list all IO conditions in ECA"
    saft-io-ctl tr0 -l
}

####################
## Make mpsrx ready
####################

setup_mpsrx() {

    check_mpsrx

    echo "load the LM32 firmware"
    eb-fwload $DEV_RX u 0x0 $FW_RX
    wait_seconds 1

    echo "CONFIGURE state "
    eb-write $DEV_RX $addr_cmd 0x1
    wait_seconds 1

    echo "set node type to DEV_RX (0x1)"
    echo "write node type (0x1) to dedicated memory location"
    eb-write $DEV_RX 0x20140820/4 0x1
    wait_seconds 1

    echo "tell LM32 to set the node type"
    eb-write $DEV_RX $addr_cmd 0x15
    wait_seconds 1

    echo "verify the actual node type"
    eb-read $DEV_RX 0x20140830/4
    wait_seconds 1

    echo "OPREADY state "
    eb-write $DEV_RX $addr_cmd 0x2

    echo "destroy all unowned ECA conditions"
    saft-ecpu-ctl tr0 -x

    echo "configure ECA: set FBAS_WR_EVT, FBAS_WR_FLG for LM32 channel, tag 0x24 and 0x25"
    saft-ecpu-ctl tr0 -c 0x1fcbfcb000000000 0xffffffff00000000 0 0x24 -d
    saft-ecpu-ctl tr0 -c 0x1fccfcc000000000 0xffffffff00000000 0 0x25 -d

    echo "configure ECA: listen for FBAS_AUX_CYCLE event, tag 0x26"
    saft-ecpu-ctl tr0 -c 0xffffdddd00000000 0xffffffff00000000 0 0x26 -d

    echo "show actual ECA conditions"
    saft-ecpu-ctl tr0 -l

    echo "disable all events from IO inputs to ECA"
    saft-io-ctl tr0 -w

    echo "list all IO conditions in ECA"
    saft-io-ctl tr0 -l
}

###################
# Tests
###################

do_inject_fbas_event() {

    saft-ctl tr0 -p inject 0xffffeeee01010000 0 1000000
}

##########################################################
# Test 4: for scuxl0497/396 in TTF
#
# TX node (scuxl0497) sends timing messages (ID=0x1fcbfcb00)
# with MPS flag. If MPS event is injected locally, then it
# sends 2x timing messages (ID=0x1fccfcc00) with MPS event
# immediatelly.
#
# RX SCU receives timing messages with MPS flag and events and
# counts number of timing messages.
#
# There is no IO connection between RX and TX nodes
##########################################################

start_test4() {
    echo -e "\nEnable MPS task on $1"

    cnt=$(eb-read $1 $addr_cnt1)           # get counter value
    cnt_dec=$(printf "%d" 0x$cnt)          # convert hex to decimal
    echo "msg_cnt: 0x$cnt (${cnt_dec})"

    enable_mps $1
}

stop_test4() {
    echo -e "\nDisable MPS task on $1"
    disable_mps $1

    cnt=$(eb-read $1 $addr_cnt1)           # get counter value
    cnt_dec=$(printf "%d" 0x$cnt)          # convert hex to to decimal
    echo "msg_cnt: 0x$cnt (${cnt_dec})"
}

##########################################################
# Test 3: measure network performance
# TX SCU sends MPS flag periodically in timing msg with event ID=0x1fcbfcb00 and
# sends MPS event immediately in timing msg with event ID=0x1fccfcc00.
#
# RX SCU drive its B1 output according to MPS flag or on time-out.
#
# IO connection with LEMO: RX:B1 -> TX:B2
##########################################################

start_nw_perf() {

    wait_seconds 1
    echo "TX: MPS events will be generated locally ..."
    echo "TX: 1x MPS event -> flag=OK(1) flag, grpID=1, evtID=0 (1x transmission)"
    saft-ctl tr0 -p inject 0xffffeeee01010000 0x0 1000000
    wait_seconds 1

    echo "TX: 1x MPS event -> flag=NOK(2), grpID=1, evtID=0 (3x transmissions)"
    saft-ctl tr0 -p inject 0xffffeeee02010000 0x0 1000000
    wait_seconds 1

    echo "TX: 10x MPS events: 5x OK and 5x NOK (20x transmissions)"
    for i in seq 1 5; do
        saft-ctl tr0 -p inject 0xffffeeee01010000 0x0 1000000
        wait_seconds 1

        saft-ctl tr0 -p inject 0xffffeeee02010000 0x0 1000000
        wait_seconds 1
    done

    echo "TX: 12x IO events must be snooped by saft-ctl, when snooper is running."

    echo "TX: send 'new cycle' in 5 seconds ..."
    wait_seconds 5
    saft-ctl tr0 -p inject 0xffffdddd00000000 0x0 1000000
    saft-ctl tr0 -p inject 0xffffdddd00000000 0x0 1000000
}

result_nw_perf() {
    # $1 - dev/wbmo
    # $2 - event counter

    cnt=$(eb-read $1 $2)
    cnt_dec=$(printf "%d" 0x$cnt)
    echo "count: 0x$cnt (${cnt_dec})"

    eb-write $1 $addr_cmd 0x32
    echo "Results of network delay measurement:"
    read_measurement_results $1 $addr_msr1
}

result_ow_delay() {
    # $1 - dev/wbmo

    eb-write $1 $addr_cmd 0x33
    echo "Results of one-way delay measurement:"
    read_measurement_results $1 $addr_msr1
}

disable_mps() {
    echo "Stop MPS on $1"
    eb-write $1 $addr_cmd 0x31
    wait_seconds 1
}

enable_mps() {
    echo "Start MPS on $1"
    eb-write $1 $addr_cmd 0x30
    wait_seconds 1
}

disable_mps_all() {
    echo "Disable MPS"
    disable_mps $DEV_TX
    disable_mps $DEV_RX
}

enable_mps_all() {
    echo "Enable MPS"
    enable_mps $DEV_RX
    enable_mps $DEV_TX
}

read_measurement_results() {
    # $1 - device (dev/wbm0)
    # $2 - shared memory location where measurement results are stored

    device=$1
    addr_msr=$2

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
    echo "avg=${avg_dec}, min=${min_dec}, max=${max_dec}, cnt=${cnt_val_dec}/${cnt_all_dec}"
}

##########################################################
# Test 2: Measure time between a signalling and TLU events (0xffffeeee00000000 and 0xffff100000000000)
#
# IO connection with LEMO: RX:B1 -> TX:B2
##########################################################

do_test2() {
    echo "injectg timing messages to DEV_TX that simulate the FBAS class 2 signals"
    saft-ctl tr0 -p inject 0xffffeeee00000000 0x0 1000000

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
