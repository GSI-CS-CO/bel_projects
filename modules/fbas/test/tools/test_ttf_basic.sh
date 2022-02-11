#!/bin/bash

# Control flow for Xenabay 'broadcast_timing_msg' and 'high_load' testbeds.
# Timing message transfer is done between 2 SCUs:
# - RX SCU (scuxl0497)
# - TX SCU (scuxl0396)

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
txscu="scuxl0396.$domain"
sleep_sec=10
unset option username userpasswd

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Run basic test to check timing message transfer between 2 SCUs."
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -y                     'yes' to all prompts"
    echo "  -h                     display this help and exit"
}

while getopts 'hyu:p:' c
do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        y) option="auto" ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '${rxscu%%.*}, ${txscu%%.*}': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

echo -e "\nset up nodes\n------------"
timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && setup_mpsrx"
timeout 10 sshpass -p "$userpasswd" ssh $username@$txscu "source setup_local.sh && setup_mpstx"

echo 'start test4 (RX, TX)'
echo "-----------"
timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && start_test4 \$DEV_RX"
timeout 10 sshpass -p "$userpasswd" ssh $username@$txscu "source setup_local.sh && start_test4 \$DEV_TX"

echo "wait $sleep_sec seconds (start Xenabay schedule now)"
echo "------------"
sleep $sleep_sec  # wait for given seconds

echo 'stop test4 (TX, RX)'
echo "----------"
timeout 10 sshpass -p "$userpasswd" ssh $username@$txscu "source setup_local.sh && stop_test4 \$DEV_TX"
timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && stop_test4 \$DEV_RX && result_ow_delay \$DEV_RX \$addr_cnt1"
