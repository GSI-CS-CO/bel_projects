#!/bin/bash

# Control flow for Xenabay 'broadcast_timing_msg' and 'high_load' testbeds.
# Timing message transfer is done between 2 SCUs:
# - RX SCU (scuxl0497)
# - TX SCU (scuxl0396)

domain=$(hostname -d)
rxscu="scuxl0497.$domain" # 00:26:7b:00:06:c5
txscu="scuxl0396.$domain" # 00:26:7b:00:06:d7
sleep_sec=10

prefix="/usr/bin"
fw_rxscu="fbas16.scucontrol.bin"      # default LM32 FW for RX SCU
script_rxscu="$prefix/setup_local.sh" # shell script on remote host

ssh_cmd="ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10"

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Run basic test to check timing message transfer between 2 SCUs."
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -y                     'yes' to all prompts"
    echo "  -v                     verbosity for the measurement results"
    echo "  -h                     display this help and exit"
}

run_remote() {
    # $1 - remote host name
    # $@ - commands for the remote host

    local host=$1; shift
    timeout 20 sshpass -p "$userpasswd" $ssh_cmd $username@$host "$@"
}

check_deployment() {
    # $1 - remote host name
    # $@ - filenames

    local scu=$1; shift
    local filenames=$@

    # keep in mind: 'filenames' is local variable, but 'filename' is remote variable
    run_remote $scu "for filename in $filenames; do \
        if [ ! -f \$filename ]; then echo "\$filename not found on $scu. Exit!"; exit 2; fi \
        done"
    ret_code=$?

    if [ $ret_code -eq 124 ]; then
        echo "access to $scu timed out. Exit!"
        exit 1
    elif [ $ret_code -ne 0 ]; then
        # one or all file(s) not found
        exit 2
    else
        # print the file info
        run_remote $scu "source setup_local.sh && for filename in $filenames; do \
            print_file_info \$filename; done"
    fi
}

main() {
    # parse script arguments
    unset option username userpasswd verbose
    unset OPTIND  # unsetting OPTIND avoids unexpected behaviour when invoking the function multiple times

    while getopts 'hyu:p:v' c
    do
        case $c in
            h) usage; exit 0 ;;
            u) username=$OPTARG ;;
            p) userpasswd=$OPTARG ;;
            y) option="auto" ;;
            v) verbose="yes" ;;
        esac
    done

    # get username and password to access SCUs
    if [ -z "$username" ]; then
        read -rp "username to access '${rxscu%%.*}, ${txscu%%.*}': " username
    fi

    if [ -z "$userpasswd" ]; then
        read -rsp "password for '$username' : " userpasswd; echo
    fi

    echo "check deployment"
    echo "----------------"

    filenames="$fw_rxscu $script_rxscu"
    scus="$rxscu $txscu"

    for scu in $scus; do
        check_deployment $scu $filenames
    done

    echo -e "\nset up nodes\n------------"

    # set up the sender options (SENDER_ALL or SENDER_TX with MAC address)
    sender_opts="SENDER_ALL"                             # default for all senders
    mac_addr=$(run_remote $txscu "eb-mon -m dev/wbm0")   # get MAC address of the sender
    ret_code=$?
    if [ $ret_code -eq 0 ]; then
        sender_opts="SENDER_TX $mac_addr"
    else
        echo "$0: warning: Could not obtain the MAC address of $txscu and it might cause following issue:"
        echo "$0: - MAC address is needed to register $txscu as sender."
        echo "$0: - Un-registered sender may not send the timing messages (EvtID: 0x1fcbfcb000000000)."
        echo "$0: - Hence, test may fail!"
    fi

    run_remote $rxscu "source setup_local.sh && setup_mpsrx $fw_rxscu $sender_opts"
    run_remote $txscu "source setup_local.sh && setup_mpstx"

    echo 'start test4 (RX, TX)'
    echo "-----------"
    run_remote $rxscu "source setup_local.sh && start_test4 \$rx_node_dev"
    run_remote $txscu "source setup_local.sh && start_test4 \$tx_node_dev"

    echo "wait $sleep_sec seconds (start Xenabay schedule now)"
    echo "------------"
    sleep $sleep_sec  # wait for given seconds

    echo 'stop test4 (TX, RX)'
    echo "----------"
    echo -n "TX: "
    run_remote $txscu "source setup_local.sh && stop_test4 \$tx_node_dev && \
        read_counters \$tx_node_dev $verbose"
    echo -n "RX: "
    run_remote $rxscu "source setup_local.sh && stop_test4 \$rx_node_dev && \
        read_counters \$rx_node_dev $verbose && result_msg_delay \$rx_node_dev $verbose"
}

export -f run_remote
export -f check_deployment

# source script, if '-s' option is given, otherwise, run script!
if [ "$1" = "-s" ]; then
    echo "sourced ${BASH_SOURCE[0]}" # do not run it!
else
    main "$@"
fi
