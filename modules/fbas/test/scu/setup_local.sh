#!/bin/sh

##########################
## Setup of the WR network
##########################
# TTF:
# WRS configured with dot-config_timing_access_fbas
# TX node: scuxl0396 (TR device=dev/wbm0)
# RX node: scuxl0497 (TR device=dev/wbm0, TLU input: B2)

# HO:
# WRS configured with dot-config_home_access_fbas
# RPi with DHCP/FTP server
# DM: Pexaria5 (TR device=dev/wbm1)
# TX node: Pexaria5 (TR device=dev/wbm0)
# RX node: Pexp (TR device=dev/wbm2, TLU input: IO2)

#####################################
## General setup for all FBAS nodes
#####################################

# node platform=SCU|PC
# node form-factor=SCU|Pexaria|Pexp
# node TR device=wbm[0|1|2]
# node TR name=tr0|fbastx|fbasrx
# node TLU input=B2|IO2
# node type=[RX|TX]_NODE

# Determine node platform
platform="SCU"
node_name=$(hostname)
if [ "$node_name" = "${node_name#scuxl}" ]; then
    platform="PC"
fi

# Declare platform-specific variables
case $platform in
    "PC")
        export node_tlu_input="IO2"
        export tx_node_dev="dev/wbm0"  # label for node device
        export rx_node_dev="dev/wbm2"
        export tx_node_name="fbastx"
        export rx_node_name="fbasrx"

        export module_dir="${PWD/fbas*/fbas}"
        export fw_dir="$module_dir/fw"
        export fw_tx="fbas128.pcicontrol.bin"
        export fw_rx="fbas128.pcicontrol.bin"
        ;;
    "SCU")
        export node_tlu_input="B2"
        export tx_node_dev="dev/wbm0"
        export rx_node_dev="dev/wbm0"
        export tx_node_name="tr0"
        export rx_node_name="tr0"

        export fw_dir="."
        export fw_tx="fbas128.scucontrol.bin"
        export fw_rx="fbas128.scucontrol.bin"
        ;;
esac

# Declare platform-specific user RAM ranges of TR
case $platform in
    "PC")
        export addr_set_node_type="0x04060694"      # user RAM range in Pexp/Pexaria
        export addr_get_node_type="0x040606a4"
        export addr_cmd="0x04060508"     # shared memory location for command buffer
        export addr_cnt="0x040607a8"     # shared memory location for transmitted message counter
        export addr_avg="0x040607dc"     # shared memory location for measurement results
        export addr_eca_vld="0x04060804" # shared memory location of counter for valid actions
        export addr_eca_ovf="0x04060808" # shared memory location of counter for overflow actions
        export addr_senderid="0x0406080c" # shared memory location of sender ID

        export mac_tx_node="0x00267b0004da" # sender ID of TX node
        ;;
    "SCU")
        export addr_set_node_type="0x20140694"     # user RAM range in SCU
        export addr_get_node_type="0x201406a4"
        export addr_cmd="0x20140508"     # shared memory location for command buffer
        export addr_cnt="0x201407a8"     # shared memory location for transmitted message counter
        export addr_avg="0x201407dc"     # shared memory location for measurement results
        export addr_eca_vld="0x20140804" # shared memory location of counter for valid actions
        export addr_eca_ovf="0x20140808" # shared memory location of counter for overflow actions
        export addr_senderid="0x2014080c" # shared memory location of sender ID
        ;;
esac

# Declare common constants

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
export instr_st_msg_dly=0x33    # store the measurement results of the messaging delay
export instr_st_sg_lty=0x34     # store the signalling latency measurement results to shared memory
export instr_st_ttl_ival=0x35   # store the TTL interval measurement results to shared memory
export instr_st_eca_handle=0x37 # store the measurement result of the ECA handling delay

export     mac_any_node="0xffffffffffff"      # MAC address of any node
# Raw event data (bits 63-16 = event ID, 15-8 = channel, 7-0 = flag)
export evt_mps_flag_any="0xffffeeee00000000"  # generator event for MPS flags
export     evt_mps_1_ok="0xffffeeee00000001"  # event to generate the MPS OK flag (1st channel)
export    evt_mps_1_nok="0xffffeeee00000002"  # event to generate the MPS NOK flag (1st channel)
export    evt_mps_1_tst="0xffffeeee00000003"  # event to generate the MPS TEST flag (1st channel)
export     evt_mps_2_ok="0xffffeeee00000101"  # event to generate the MPS OK flag (2nd channel)
export    evt_mps_2_nok="0xffffeeee00000102"  # event to generate the MPS NOK flag (2nd channel)
export    evt_mps_2_tst="0xffffeeee00000103"  # event to generate the MPS TEST flag (2nd channel)
export evt_mps_prot_std="0x1fcbfcb000000000"  # event with MPS protocol (regular)
export evt_mps_prot_chg="0x1fccfcc000000000"  # event with MPS protocol (change in flag)
export          evt_tlu="0xffff100000000000"  # TLU event (used to catch the signal change at IO port)
export    evt_new_cycle="0xffffdddd00000000"  # event for new cycle
export evt_mps_node_reg="0x1fcdfcd000000000"  # event ID for the node registration messages
export      evt_id_mask="0xffffffff00000000"  # event mask

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

start_saftd() {
    echo "terminate SAFT daemon if it's running"
    sudo killall saftd

    if [ $? -eq 0 ]; then
        echo "wait until SAFT daemon terminates"
        for i in $(seq 1 10); do
            echo -ne "time left (seconds): $[ 10 - $i ]\r"
            wait_seconds 1
        done
    fi

    case $platform in
        "PC")
            echo "attach $tx_node_name:$tx_node_dev and $rx_node_name:$rx_node_dev'"
            sudo saftd $tx_node_name:$tx_node_dev $rx_node_name:$rx_node_dev
            ;;
        "SCU")
            ;;
    esac
}

start_saftbusd() {
    echo "terminate saftbus daemon, if it's running"
    sudo killall saftbusd

    if [ $? -eq 0 ]; then
        echo "wait until watchdog is released"
        for i in $(seq 1 10); do
            echo -ne "time left (seconds): $[ 10 - $i ]\r"
            wait_seconds 1
        done
    fi

    case $platform in
        "PC")
            export SAFTBUS_SOCKET_PATH=/tmp/saftbus

            echo "start saftbus with $tx_node_name:$tx_node_dev and $rx_node_name:$rx_node_dev'"
            saftbusd libsaft-service.so $tx_node_name:$tx_node_dev $rx_node_name:$rx_node_dev &
            ;;
        "SCU")
            ;;
    esac
}

check_node() {
    # $1 - node device label

    if [ "$1" != "tx_node_dev" ] && [ "$1" != "rx_node_dev" ]; then
        echo "unknown label for node device: $1 -> Exit!"
        exit 1
    fi
}

load_node_fw() {
    # $1 - node device label
    # $2 - firmware filename

    check_node "$1"

    unset device fw_filename

    if [ "$1" == "rx_node_dev" ]; then
        fw_filename=$fw_dir/$fw_rx
    else
        fw_filename=$fw_dir/$fw_tx
    fi

    if [ ! -z "$2" ]; then
        fw_filename="$fw_dir/$2"
        if [ ! -f "$fw_filename" ]; then
            echo "'$fw_filename' not found. Exit"
            return 1
        fi
    fi

    device=$(eval echo "\$$1") # reference node device label (string) as variable

    echo "$1 ($device): load the LM32 firmware '$fw_filename'"
    eb-fwload $device u 0x0 $fw_filename
    if [ $? -ne 0 ]; then
        echo "Error: failed to load LM32 FW '$fw_filename'. Exit!"
        exit 1
    fi
    wait_seconds 1
}

configure_node() {
    # $1 - node device label
    # $2 - sender node groups (SENDER_TX or SENDER_ANY or SENDER_ALL)
    # $[3:] - sender ID(s) of SENDER_TX

    check_node "$1"

    device=$(eval echo "\$$1")  # reference node device label (string) as variable

    eb-write $device $addr_cmd/4 $instr_fsm_configure
    wait_seconds 1

    if [ "$1" == "rx_node_dev" ]; then
        echo "set node type to RX (0x1)"
        eb-write $device $addr_set_node_type/4 0x1
        wait_seconds 1

        echo "tell LM32 to set the node type"
        eb-write $device $addr_cmd/4 $instr_set_nodetype
        wait_seconds 1

        echo "verify the actual node type (expected 0x1)"
        eb-read $device $addr_get_node_type/4
        wait_seconds 1

        if [ $# -gt 1 ]; then
            shift
            set_senderid "$device" "$@"
        fi
    fi
}

set_senderid() {
    # $1 - node device
    # $2 - sender groups, valid values: SENDER_TX, SENDER_ANY, SENDER_ALL
    # $[3:] - sender ID(s) of SENDER_TX (without leading 0x)

    # SENDER_TX - only TX node
    # SENDER_ANY - only any nodes
    # SENDER_ALL - TX and any nodes

    device=$1
    sender_grp="$2"
    shift                   # re-set positional parameters
    shift
    #set -- "${@/#/0x}"      # add prefix (0x) to all positional parameters
    unset senderid          # init variable
    for mac in "$@"; do
        senderid="$senderid 0x$mac"   # format to hexadecimal number (for arithmetic calc.)
    done

    first_idx=1
    last_idx=15
    unset idx_mac_list  # list with idx_mac

    if [ "$sender_grp" == "SENDER_TX" ]; then
        # Structure of the MPS message buffer: 'sender id', 'index' and 'MPS flag'.
        # The buffer can keep the MPS flag of up to 16 TX nodes.
        # The 'index' is used to identify channels of the same sender, therefore
        # it's set to zero if each sender has only one MPS channel.
        i=0
        for sender in $senderid; do
            idx=$(( $i << 48 ))
            idx_mac=$(( $idx + $sender ))
            idx_mac=$(printf "0x%x" $idx_mac)
            idx_mac_list="$idx_mac_list $idx_mac"
        done
    elif [ "$sender_grp" == "SENDER_ALL" ] || [ "$sender_grp" == "SENDER_ANY" ]; then
        if [ "$sender_grp" == "SENDER_ALL" ]; then
            idx_mac_list="$senderid"
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

    echo "set the sender IDs: $sender_grp $idx_mac_list"

    i=0
    for idx_mac in $idx_mac_list; do
        pos=$(( $i << 56 ))                               # position in RX buffer
        senderid=$(( $pos + $idx_mac ))                   # sender ID = position + (idx + MAC)
        senderid=$(printf "0x%x" $senderid)
        addr_id=$addr_senderid
        id_32=$(($senderid >> 32))                        # high 32-bit of sender ID
        eb-write $device $addr_id/4 $id_32
        addr_id=$(($addr_id + 4))
        id_32=$(($senderid & 0xffffffff))                 # low 32-bit of sender ID
        eb-write $device $addr_id/4 $id_32

        eb-write $device $addr_cmd/4 $instr_load_senderid
        i=$(( $i + 1 ))
        sleep 0.2
    done

}

make_node_ready() {
    # $1 - node device label

    check_node "$1"

    device=$(eval echo "\$$1")  # reference node device label (string) as variable

    eb-write $device $addr_cmd/4 $instr_fsm_opready
    wait_seconds 1
}

configure_eca() {
    # $1 - node device label

    check_node "$1"

    set_eca_rules $1
}

set_eca_rules() {
    # $1 - node device label

    case $1 in
        "tx_node_dev")
            node_name=$tx_node_name
            node_device=$tx_node_dev
            ;;
        "rx_node_dev")
            node_name=$rx_node_name
            node_device=$rx_node_dev
            ;;
    esac

    # configure ECA: general rules for both nodes
    echo "destroy all unowned ECA conditions"
    saft-ecpu-ctl $node_name -x

    echo "disable all events from IO inputs to ECA"
    saft-io-ctl $node_name -w

    echo "configure ECA ($node_name): listen to the node registration messages ($evt_mps_node_reg), tag 0x45"
    saft-ecpu-ctl $node_name -c $evt_mps_node_reg $evt_id_mask 0 0x45 -d

    echo "configure ECA ($node_name): listen for FBAS_AUX_CYCLE event ($evt_new_cycle), tag 0x26"
    saft-ecpu-ctl $node_name -c $evt_new_cycle $evt_id_mask 0 0x26 -d

    if [ "$1" == "tx_node_dev" ]; then
        echo "configure ECA ($node_name): set FBAS_GEN_EVT ($evt_mps_flag_any) for LM32 channel, tag 0x42"
        saft-ecpu-ctl $node_name -c $evt_mps_flag_any $evt_id_mask 0 0x42 -d

        echo "configure ECA ($node_name): listen for TLU event ($evt_tlu), tag 0x43"
        saft-ecpu-ctl $node_name -c $evt_tlu $evt_id_mask 0 0x43 -d

        echo "configure TLU ($node_name): timing event ($evt_tlu) is generated on signal transition at $node_tlu_input input"
        saft-io-ctl $node_name -n $node_tlu_input -b $evt_tlu

        echo "now events can be snooped with a following command: saft-ctl $node_name -xv snoop 0 0 0"
        echo "or events can be presented by the LM32 firmware if WR console is active: $ eb-console $node_device"
    else
        echo "configure ECA ($node_name): set FBAS_WR_FLG ($evt_mps_prot_std), FBAS_WR_EVT ($evt_mps_prot_chg) for LM32 channel, tag 0x24 and 0x25"
        saft-ecpu-ctl $node_name -c $evt_mps_prot_std $evt_id_mask 0 0x24 -d
        saft-ecpu-ctl $node_name -c $evt_mps_prot_chg $evt_id_mask 0 0x25 -d
    fi

    echo "show actual ECA conditions"
    saft-ecpu-ctl $node_name -l

    echo "list all IO conditions in ECA"
    saft-io-ctl $node_name -l
}

######################
## Make 'mpstx' ready
######################

setup_mpstx() {

    node_dev_label="tx_node_dev"
    echo "load firmware"

    load_node_fw "$node_dev_label"

    echo "CONFIGURE state "
    configure_node "$node_dev_label"

    echo "OPREADY state "
    make_node_ready "$node_dev_label"

    echo "configure TR"
    configure_eca "$node_dev_label"
}

####################
## Make mpsrx ready
####################

setup_mpsrx() {
    # $1 - LM32 firmware
    # $2 - sender node groups (SENDER_TX/ALL/ANY)
    # $[3:] - sender ID(s) of SENDER_TX

    node_dev_label="rx_node_dev"

    echo "load firmware"
    load_node_fw "$node_dev_label" "$1"

    echo "CONFIGURE state "
    shift
    configure_node "$node_dev_label" "$@"

    echo "OPREADY state "
    make_node_ready "$node_dev_label"

    echo "configure TR"
    configure_eca "$node_dev_label"
}

###################
# Tests
###################

do_inject_fbas_event() {

    saft-ctl tr0 -p inject $evt_mps_1_tst 0 1000000
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

read_counters() {
    # $1 - dev/wbm0
    # $2 - verbosity

    device=$1
    verbose=$2
    addr_val="$addr_cnt $addr_eca_vld $addr_eca_ovf" # reg addresses as string
    unset counts

    for addr in $addr_val; do
        cnt=$(eb-read $device $addr/4)  # get counter value
        cnt_dec=$(printf "%d" 0x$cnt)
        counts="${counts}$cnt_dec "
    done
    if [ -n "$verbose" ]; then
        counts="${counts}(tx_msg rx_vld rx_ovf)\n"
    else
        counts="${counts}\n"
    fi

    echo -e "$counts"
}

start_test4() {
    # $1 - dev/wbm0

    echo -e "\nEnable MPS task on $1"
    enable_mps $1
}

stop_test4() {
    # $1 - dev/wbm0

    echo -e "\nDisable MPS task on $1"
    disable_mps $1
}

info_nw_perf() {
    # $1 - number of iterations

    n=$1

    echo "TX: generating the MPS events locally ..."
    echo "TX: $n events ($evt_mps_1_nok, flag=NOK(2), $((3 * n)) MPS msgs)"
    echo "TX: $n events ($evt_mps_2_nok, flag=NOK(2), $((3 * n)) MPS msgs)"
    echo "TX: $n events ($evt_mps_1_ok, flag=OK(1), $n msgs)"
    echo "TX: $n events ($evt_mps_2_ok, flag=OK(1), $n msgs)"
    echo -e "TX: $(( n * 4 - 2 ))x IO events must be snooped by 'saft-ctl tr0 -vx snoop $evt_tlu $evt_id_mask 0'\n"
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
    # $1 - number of iterations

    n=10
    if [ -n "$1" ]; then
        n=$1
    fi

    # info_nw_perf $n

    param=0
    offset_ns=0
    for i in $(seq $n); do

        echo -en " $i: NOK\r"
        saft-ctl tr0 inject $evt_mps_1_nok $param $offset_ns
        sleep 0.5
        saft-ctl tr0 inject $evt_mps_2_nok $param $offset_ns
        sleep 0.5

        echo -en " $i:  OK\r"
        saft-ctl tr0 inject $evt_mps_1_ok $param $offset_ns
        sleep 0.5
        saft-ctl tr0 inject $evt_mps_2_ok $param $offset_ns
        sleep 0.5
    done
}

result_event_count() {
    # sent/received event count

    # $1 - dev/wbmo
    # $2 - event counter
    # $3 - verbosity

    cnt=$(eb-read $1 $2/4)
    cnt_dec=$(printf "%d" 0x$cnt)
    if [ -n "$3" ]; then
        echo -n "count: "
    fi
    echo "0x$cnt (${cnt_dec})"
}

result_tx_delay() {
    # $1 - dev/wbmo
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "TX dly: "
    fi
    read_measurement_results $1 $instr_st_tx_dly $addr_avg $2
}

result_sg_latency() {
    # $1 - dev/wbmo
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "MPS lty: "
    fi
    read_measurement_results $1 $instr_st_sg_lty $addr_avg $2
}

result_msg_delay() {
    # $1 - dev/wbmo
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "Msg dly: "
    fi
    read_measurement_results $1 $instr_st_msg_dly $addr_avg $2
}

result_ttl_ival() {
    # $1 - dev/wbmo
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "TTL ivl: "
    fi
    read_measurement_results $1 $instr_st_ttl_ival $addr_avg $2
}

result_eca_handle() {
    # $1 - dev/wbmo
    # $2 - verbosity

    if [ -n "$2" ]; then
        echo -n "ECA hdl: "
    fi
    read_measurement_results $1 $instr_st_eca_handle $addr_avg $2
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
    disable_mps $tx_node_dev
    disable_mps $rx_node_dev
}

enable_mps_all() {
    echo "Enable MPS"
    enable_mps $rx_node_dev
    enable_mps $tx_node_dev
}

eb_read_uint64() {
    # $1 - device (dev/wbm0)
    # $2 - address (0x2014000)

    device=$1
    addr=$2

    val_hi="0x$(eb-read $device ${addr}/4)" # read and keep value as hex
    addr=$(($addr + 4))
    val_lo="0x$(eb-read $device ${addr}/4)"
    val=$(($val_hi << 4))
    val=$(($val + $val_lo))
    echo "$val"
}

read_measurement_results() {
    # $1 - node device (dev/wbm0)
    # $2 - instruction code to store measurement results to a location in the shared memory
    # $3 - shared memory location where measurement results are stored
    # $4 - verbosity

    device=$1
    instr_msr=$2
    addr_msr=$3

    eb-write $device $addr_cmd/4 $instr_msr

    avg=$(eb_read_uint64 $device $addr_msr)  # average
    avg=$(($avg / 1000))                     # ns->us

    addr_msr=$(( $addr_msr + 8 ))
    min=$(eb_read_uint64 $device $addr_msr)
    min=$(printf "%lli" $min)                # min might be negative
    min=$(($min / 1000))                     # ns->us

    addr_msr=$(( $addr_msr + 8 ))
    max=$(eb_read_uint64 $device $addr_msr)  # max
    max=$(($max / 1000))                     # ns->us

    addr_msr=$(( $addr_msr + 8 ))
    cnt_val=$(eb-read $device ${addr_msr}/4)
    cnt_val=$(printf "%d" 0x$cnt_val)        # valid, converted to decimal

    addr_msr=$(( $addr_msr + 4 ))
    cnt_all=$(eb-read $device ${addr_msr}/4)
    cnt_all=$(printf "%d" 0x$cnt_all)        # all

    echo -n "${avg} ${min} ${max} ${cnt_val} ${cnt_all}"
    if [ -n "$4" ]; then
        echo " (avg min max [us] valid all)"
    else
        echo
    fi
}

##########################################################
# Test 2: Measure time between a signalling and TLU events ($evt_mps_flag_any and $evt_tlu)
#
# IO connection with LEMO: RX:B1 -> TX:B2
##########################################################

do_test2() {
    echo "injectg timing messages to tx_node_dev that simulate the FBAS class 2 signals"
    saft-ctl $tx_node_name -p inject $evt_mps_1_tst 0x0 1000000

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

####################
## Reset FTRN node
#
# - FW is not re-loaded
# + reset LM32
# + set node type, if 'RX'
# + set oper. mode to 'OPREADY'
####################

reset_node() {
    # $1 - node device label
    # $[2:] - sender node groups

    check_node "$1"

    device=$(eval echo "\$$1") # reference node device label (string) as variable

    echo "reset LM32"
    eb-reset $device cpureset 0x0
    sleep 1

    echo "CONFIGURE state "
    configure_node "$@"

    echo "OPREADY state "
    make_node_ready "$1"

    case $1 in
        "tx_node_dev")
            node_name=$tx_node_name
            ;;
        "rx_node_dev")
            node_name=$rx_node_name
            ;;
    esac

    echo "show actual ECA conditions"
    saft-ecpu-ctl $node_name -l

    echo "list all IO conditions in ECA"
    saft-io-ctl $node_name -l
}

###################
# Inject the MPS events locally
#
# Use saft-dm to inject the MPS events
###################
inject_mps_events() {
    # $1 - iteration of schedule
    # $2 - filename with the MPS event schedule

    saft-dm bla -fp -n $1 $2
}

###################
# Print the MD5 checksum and date of a given file
#
###################
print_file_info() {
    # $1 - filename (in current directory)

    if [ -f $1 ]; then
        checksum=$(md5sum $1)
        info=$(ls -l $1)

        checksum=${checksum%% *} # get the checksum only
        date=${info##*root}      # get the sub-string right to 'root'

        printf "%s " $checksum $date
        printf "\n"

    else
        exit 2                   # return error code
    fi

}
