#!/bin/bash

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu_name="scuxl0497"
def_txscu_name="scuxl0396"
txscu_name=()                         # array with transmitter names
rxscu="scuxl0497.$domain"
txscu=()                              # array with transmitter domain names
fw_scu_def="fbas128.scucontrol.bin"   # FW that supports up to 16 TX nodes, each has 8 MPS channels
fw_scu_multi="fbas128.scucontrol.bin"
ssh_opts="-o StrictHostKeyChecking=no"   # no hostkey checking
getopt_opts="u:p:t:r:n:eyvh"          # user options

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
    echo "  -n <MPS events>        number of MPS events, 10 by default"
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
    echo "TX: events expected, when B1 output is driven on RX:"
    echo "    GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001"
    echo "    GID: 0x0fff EVTNO: 0x0100 Other: 0x000000000"

    echo "RX: drive B1 output:"
    echo "      saft-io-ctl tr0 -n B1 -o 1 -d 1"
    echo "      saft-io-ctl tr0 -n B1 -o 1 -d 0"
}

setup_nodes() {

    filenames="$fw_scu_def $fw_scu_multi $script_rxscu"
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
        for filename in $filenames; do
            run_remote $scu "source setup_local.sh && print_file_info $filename"
            ret_code=$?

            if [ $ret_code -eq 124 ]; then
                echo "access to $scu timed out. Exit!"
                exit 1
            elif [ $ret_code -ne 0 ]; then
                echo "$filename not found on ${scu}. Exit!"
                exit 2
            fi
        done

        # set up TX nodes
        if [ "$scu" != "$rxscu" ]; then
            output=$(run_remote $scu "source setup_local.sh && setup_mpstx")
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

    output=$(run_remote $rxscu "source setup_local.sh && setup_mpsrx $fw_scu_multi $sender_opts")
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
    output=$(source $dir_name/../scu/setup_local.sh && info_nw_perf $events)
    echo -e "$output\n"

    # enable simultaneous operation of TX nodes
    pids=()

    for i in ${!txscu[@]}; do
        echo ${txscu[$i]}

        # enable MPS operation, start test => keep process ID
        output=$(run_remote ${txscu[$i]} "source setup_local.sh && enable_mps \$tx_node_dev")

        # start test sub-process and keep its process ID
        run_remote ${txscu[$i]} "source setup_local.sh && start_nw_perf $events" &
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
        output=$(run_remote $scu "source setup_local.sh && disable_mps \$tx_node_dev")
    done

    output=$(run_remote $rxscu "source setup_local.sh && disable_mps \$rx_node_dev")

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
        tx_delay_entries=("sig lty" "tx  dly" "mps hdl")
        rx_delay_entries=("msg dly" "ttl    " "eca hdl")
    fi

    i=0
    for scu in ${txscu[@]}; do
        echo "TX (${scu%%.*}):"

        # read command output line by line
        run_remote $scu \
            "source setup_local.sh && \
            result_sg_latency \$tx_node_dev $verbose && \
            result_tx_delay \$tx_node_dev $verbose && \
            result_eca_handle \$tx_node_dev $verbose" |
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
        result_msg_delay \$rx_node_dev $verbose && \
        result_ttl_ival \$rx_node_dev $verbose && \
        result_eca_handle \$rx_node_dev $verbose" |
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
    echo -e "start the measurement\n"
    output=$(run_remote $rxscu "source setup_local.sh && enable_mps \$rx_node_dev")

    n_toggle=10
    echo -e "toggle MPS operation (n=$n_toggle): TX=${txscu_name[@]}"
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

unset username userpasswd events exclude_ttl auto verbose
unset OPTIND

while getopts $getopt_opts c; do
    case $c in
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        t) txscu_name+=("$OPTARG"); txscu+=("$OPTARG.$domain") ;;
        r) rxscu_name=$OPTARG; rxscu=$OPTARG.$domain ;;
        n) events=$OPTARG ;;
        e) exclude_ttl="exclude_ttl" ;;
        y) auto="auto" ;;
        v) verbose="yes" ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$rxscu_name, ${txscu_name[@]}': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

# get the default transmitter SCU name
if [ ${#txscu_name[@]} -eq 0 ]; then
    txscu_name+=("$def_txscu_name")
    txscu+=("$def_txscu_name.$domain")
fi

# set the number of events
if [ -z "$events" ]; then
    events=10
fi

echo -e "\n--- Step 1: set up nodes (RX=$rxscu_name, TX=${txscu_name[@]}) ---\n"
setup_nodes

# optional pre-check before real test
echo -e "\n--- Step 2: pre-check (RX=$rxscu_name, TX=${txscu_name[@]}) ---\n"
pre_check

if [ -z "$auto" ]; then
    user_approval
fi

echo -e "\n--- Step 3: measure network performance (RX=$rxscu_name, TX=${txscu_name[@]}) ---\n"
measure_nw_perf

if [ -n "$exclude_ttl" ]; then
    exit 0
fi

# TTL measurement
echo -e "\n--- Step 4: measure TTL (RX=$rxscu_name, TX=${txscu_name[@]}) ---\n"
measure_ttl
