#!/bin/bash

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu_name="scuxl0497"
def_txscu_name="scuxl0396"
txscu_name=()                         # array with transmitter names
rxscu="$rxscu_name.$domain"
txscu=()                              # array with transmitter domain names
fw_scu_def="fbas128.scucontrol.bin"   # FW that supports up to 16 TX nodes, each has 8 MPS channels
ssh_opts="-o StrictHostKeyChecking=no"   # no hostkey checking
getopt_opts="u:p:t:r:g:m:eyvh"        # user options

usage() {

    echo "Run a MPS signalling test between TX and RX SCUs."
    echo "User might be logged in to both SCUs to perform optional pre-check."
    echo
    echo "Usage: $0 [options]"
    echo
    echo " options:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpassd>         user password"
    echo "  -t <TX SCU>            transmitter SCU, by default $def_txscu_name"
    echo "  -r <RX SCU>            receiver SCU, by default $rxscu_name"
    echo "  -g <event gen period>  pseudo event generation period (10 seconds by default)"
    echo "  -m <messaging index>   index of the TX messaging period (0..8, 0 = 33,3 ms)"
    echo "  -e                     exclude TTL measurement"
    echo "  -y                     'yes' to all prompts"
    echo "  -v                     verbosity for the measurement results"
    echo "  -h                     display this help and exit"
}

user_approval() {
    echo -en "\nCONITNUE (Y/n)? "
    read -r answer

    if [ "$answer" != "y" ] && [ "$answer" != "Y" ] && [ -n "$answer" ]; then
        exit 1
    fi
}

pre_check() {
    echo "TX: snoop TLU event (for IO action):"
    echo "    saft-ctl tr0 -xv snoop 0 0 0"
    echo "TX: expected events at a chosen input port (B2/IO2):"
    echo "    GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001"
    echo "    GID: 0x0fff EVTNO: 0x0100 Other: 0x000000000"
}

setup_nodes() {

    filenames="$fw_scu_def $script_rxscu"

    mac_txscu=()
    all_scu=(${txscu[@]} "$rxscu")

    for scu in "${all_scu[@]}"; do
        # get MAC address
        mac_scu=$(run_remote $scu "eb-mon -m dev/wbm0")
        ret_code=$?
        if [ $ret_code -eq 0 ]; then
            if [ "$scu" != "$rxscu" ]; then
                mac_txscu+=($mac_scu)
                echo "TX=$scu: $mac_scu"
            else
                echo "RX=$scu: $mac_scu"
            fi
        fi

        # check deployment
        check_deployment $scu $filenames

        # set up TX nodes
        if [ "$scu" != "$rxscu" ]; then
            output=$(run_remote $scu "source setup_local.sh && setup_mpstx $idx_msg_period")
            ret_code=$?
            if [ $ret_code -ne 0 ]; then
                echo "Error ($ret_code): cannot set up $scu"
                exit 1
            fi
        fi
        echo
    done

    # set up RX node
    sender_opts="SENDER_ANY"
    if [ ${#mac_txscu[@]} -eq 0 ]; then
        sender_opts="SENDER_ANY"
    else
        sender_opts="SENDER_TX ${mac_txscu[@]}"
    fi

    output=$(run_remote $rxscu "source setup_local.sh && setup_mpsrx $fw_scu_def $sender_opts")
    ret_code=$?
    if [ $ret_code -ne 0 ]; then
        echo "Error ($ret_code): cannot set up $rxscu_name"
        exit 1
    fi

    echo "Sender ID(s): ${mac_txscu[@]}"
}

measure_nw_perf() {
    echo -e "start the measurements\n"
    output=$(run_remote $rxscu "source setup_local.sh && enable_mps \$rx_node_dev")

    # use local script to print info
    output=$(source $dir_name/../scu/setup_local.sh && info_nw_perf $gen_period)
    echo -e "$output\n"

    # enable simultaneous operation of TX nodes
    pids=()

    for i in ${!txscu[@]}; do
        echo ${txscu[$i]}

        # enable MPS operation, start test => keep process ID
        output=$(run_remote ${txscu[$i]} "source setup_local.sh && enable_mps \$tx_node_dev")

        # start test sub-process and keep its process ID
        run_remote ${txscu[$i]} "source setup_local.sh && start_nw_perf $gen_period" &
        pids[$i]=$!
    done

    # notice the start time (after calling sub-process in the last device)
    start=$(date +%s)

    # wait until all sub-processes are complete
    for pid in ${pids[@]}; do
        wait $pid
    done

    # notice the end time (sub-process is finished in the last device)
    end=$(date +%s)
    # calculate the runtime
    runtime=$((end - start))

    echo -e "stop the measurements, runtime $runtime seconds\n"
    for scu in ${txscu[@]}; do
        output=$(run_remote $scu "source setup_local.sh && stop_operation \$tx_node_dev")
    done

    output=$(run_remote $rxscu "source setup_local.sh && stop_operation \$rx_node_dev")

    # report test result
    echo "measurement stats of MPS signaling"
    if [ -z "$verbose" ]; then
        echo -e "MPS node:       tx_cnt rx_vld rx_ovf\n"
    else
        echo
    fi

    sum_tx_cnt=0
    for scu in ${txscu[@]}; do
        cnt=$(run_remote $scu \
            "source setup_local.sh && \
            read_counters \$tx_node_dev $verbose")
        echo "TX (${scu%%.*}): $cnt"
	tx_cnt=${cnt%% *} # remove the right part including all spaces
	sum_tx_cnt=$(( $sum_tx_cnt + $tx_cnt ))
    done

    cnt=$(run_remote $rxscu \
        "source setup_local.sh && \
        read_counters \$rx_node_dev $verbose")
    echo "RX (${rxscu%%.*}): $cnt"

    rx_cnt=${cnt#* }     # remove the left part including first occurence of space
    rx_cnt=${rx_cnt%% *} # remove the right part including all spaces

    result="received $rx_cnt of $sum_tx_cnt"
    if [ $rx_cnt -eq $sum_tx_cnt ]; then
	echo -e "PASS: $result\n"
    else
	echo -e "FAIL: $result\n"
    fi

    # print measurement header
    if [ -z "$verbose" ]; then
        echo -e "Delay:  avg min max [us] vld all [])\n"
        # declare measurement entries
        tx_delay_entries=("eca dly" "tx dly " "ml prd ")
        rx_delay_entries=("eca dly" "rx dly " "msg dly" "ml prd " "ttl    ")
    fi

    i=0
    for scu in ${txscu[@]}; do
        echo "TX (${scu%%.*}):"

        # read command output line by line
        run_remote $scu \
            "source setup_local.sh && \
            result_eca_delay \$tx_node_dev $verbose && \
            result_tx_delay \$tx_node_dev $verbose && \
            result_ml_period \$tx_node_dev $verbose" |
        while IFS= read -r line; do
            delay_entry="${tx_delay_entries[$i]}"
            if [ -n "$delay_entry" ]; then
                delay_entry=" $delay_entry:"
            fi
            echo "$delay_entry $line"
            i=$((i + 1))
        done
    done

    i=0
    echo "RX (${rxscu%%.*}):"
    run_remote $rxscu \
        "source setup_local.sh && \
        result_eca_delay \$rx_node_dev $verbose && \
        result_rx_delay \$rx_node_dev $verbose && \
        result_msg_delay \$rx_node_dev $verbose && \
        result_ml_period \$rx_node_dev $verbose && \
        result_ttl_ival \$rx_node_dev $verbose" |
    while IFS= read -r line; do
        delay_entry="${rx_delay_entries[$i]}"
        if [ -n "$delay_entry" ]; then
            delay_entry=" $delay_entry:"
        fi
        echo "$delay_entry $line"
        i=$((i +  1))
    done
}

measure_ttl() {
    echo -e "RX: start the FW operation\n"
    output=$(run_remote $rxscu "source setup_local.sh && start_operation \$rx_node_dev")

    echo -e "TX: start the FW operation: TX=${txscu_name[*]}"
    for scu in ${txscu[@]}; do
        output=$(run_remote $scu "source setup_local.sh && start_operation \$tx_node_dev")
    done

    echo -e "start the measurement\n"
    output=$(run_remote $rxscu "source setup_local.sh && enable_mps \$rx_node_dev")

    n_toggle=10
    echo -e "toggle MPS operation (n=$n_toggle): TX=${txscu_name[*]}"
    for i in $(seq 1 $n_toggle); do
        echo -en " $i: enable \r"

        for scu in ${txscu[@]}; do
            output=$(run_remote $scu "source setup_local.sh && enable_mps \$tx_node_dev")
        done

        sleep 1
        echo -en " $i: disable\r"

        for scu in ${txscu[@]}; do
            output=$(run_remote $scu "source setup_local.sh && disable_mps \$tx_node_dev")
        done

        sleep 1
    done

    echo -e "\nstop the measurement\n"
    output=$(run_remote $rxscu "source setup_local.sh && disable_mps \$rx_node_dev")

    echo -e "measurement stats: TTL\n"
    run_remote $rxscu "source setup_local.sh && result_ttl_ival \$rx_node_dev \$addr_cnt1 $verbose"
}

unset username userpasswd gen_period idx_msg_period exclude_ttl auto verbose
unset OPTIND

while getopts $getopt_opts c; do
    case $c in
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        t) txscu_name+=("$OPTARG"); txscu+=("$OPTARG.$domain") ;;
        r) rxscu_name=$OPTARG; rxscu=$OPTARG.$domain ;;
        g) gen_period=$OPTARG ;;
        m) idx_msg_period=$OPTARG ;;
        e) exclude_ttl="exclude_ttl" ;;
        y) auto="auto" ;;
        v) verbose="yes" ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done

# check the index of the TX messaging period
if [ "$idx_msg_period" ]; then
    num=$(($idx_msg_period)) 2>/dev/null # 0..8
    if [ $num -gt 8 ]; then
        echo "Error: invalid index for TX messaging period: $num (expects 0..8). Exit!"
        usage; exit 1
    fi
fi

# get the default transmitter SCU name
if [ ${#txscu_name[@]} -eq 0 ]; then
    txscu_name+=("$def_txscu_name")
    txscu+=("$def_txscu_name.$domain")
fi

scu_names="$rxscu_name, ${txscu_name[*]}"
# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$scu_names': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username@{$scu_names}': " userpasswd; echo
fi

# set the pseudo event generation period
if [ -z "$gen_period" ]; then
    gen_period=10
fi

echo -e "\n--- Step 1: set up nodes (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"
setup_nodes

# optional pre-check before real test
echo -e "\n--- Step 2: pre-check (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"
pre_check

if [ -z "$auto" ]; then
    user_approval
fi

echo -e "\n--- Step 3: measure network performance (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"
measure_nw_perf

if [ -n "$exclude_ttl" ]; then
    exit 0
fi

# TTL measurement
echo -e "\n--- Step 4: measure TTL (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"
measure_ttl
