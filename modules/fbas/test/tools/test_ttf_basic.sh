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

report_check() {
    # $1 - result
    # $2 - filename
    # $3 - remote host name

    if [ $1 -eq 124 ]; then
        echo "access to $3 timed out. Exit!"
        exit 1
    elif [ $1 -ne 0 ]; then
        echo "$2 not found on ${3}. Exit!"
        exit 2
    else
        echo "$2 is available on $3"
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
        read -rsp "password for '$username' : " userpasswd
    fi

    echo "check deployment"
    echo "----------------"

    filenames="$fw_rxscu $script_rxscu"

    for filename in $filenames; do
        timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "if [ ! -f $filename ]; then echo $filename not found on ${rxscu}; exit 2; fi"
        result=$?
        report_check $result $filename $rxscu
    done

    echo -e "\nset up nodes\n------------"
    timeout 20 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_ALL"
    timeout 20 sshpass -p "$userpasswd" ssh $username@$txscu "source setup_local.sh && setup_mpstx"

    echo 'start test4 (RX, TX)'
    echo "-----------"
    timeout 20 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && start_test4 \$rx_node_dev"
    timeout 20 sshpass -p "$userpasswd" ssh $username@$txscu "source setup_local.sh && start_test4 \$tx_node_dev"

    echo "wait $sleep_sec seconds (start Xenabay schedule now)"
    echo "------------"
    sleep $sleep_sec  # wait for given seconds

    echo 'stop test4 (TX, RX)'
    echo "----------"
    echo -n "TX: "
    timeout 20 sshpass -p "$userpasswd" ssh $username@$txscu "source setup_local.sh && stop_test4 \$tx_node_dev && \
        read_counters \$tx_node_dev $verbose"
    echo -n "RX: "
    timeout 20 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && stop_test4 \$rx_node_dev && \
        read_counters \$rx_node_dev $verbose && result_ow_delay \$rx_node_dev $verbose"
}

export -f report_check

while getopts 's' c
do
    case $c in
        s) echo "sourced ${BASH_SOURCE[0]}" ;;  # source script, do not run it!
        *) main "$@" ;;                         # run script
    esac
done
