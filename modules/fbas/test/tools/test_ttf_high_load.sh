#!/bin/bash

# Control flow dedicated for the Xenabay 'high_load' testbed.
# RX SCU - scuxl0497

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
sleep_sec=20
fw_rxscu="fbas128.scucontrol.bin"    # default LM32 FW for RX SCU

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Control procedure dedicated for the Xenabay 'high_load' testbed."
    echo "Used SCUs: ${rxscu%%.*} (RX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -v                     enable verbosity"
    echo "  -h                     display this help and exit"
}

unset username userpasswd verbose
unset OPTIND

while getopts 'hu:p:vs' c; do
    case $c in
        h) usage; exit 1 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        v) verbose="yes" ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '${rxscu%%.*}: " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

echo "check deployment"
echo "----------------"

filenames="$fw_rxscu $script_rxscu"

check_deployment $rxscu $filenames

echo -e "\nset up '${rxscu%%.*}'\n------------"
output=$(run_remote $rxscu "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_ALL")

# enable MPS task of rxscu
run_remote $rxscu "source setup_local.sh && start_test4 \$rx_node_dev"

echo "wait $sleep_sec seconds (start Xenabay schedule now)"
echo "------------"
sleep $sleep_sec  # wait for given seconds

# disable MPX task of rxscu"
run_remote $rxscu "source setup_local.sh && stop_test4 \$rx_node_dev && \
    read_counters \$rx_node_dev $verbose && result_msg_delay \$rx_node_dev $verbose"
