#!/bin/bash

# Control flow dedicated for the Xenabay 'high_load' testbed.
# RX SCU - scuxl0497

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
sleep_sec=20
fw_rxscu="fbas16.scucontrol.bin"    # default LM32 FW for RX SCU

unset username userpasswd verbose

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Control procedure dedicated for the Xenabay 'high_load' testbed."
    echo "Used SCUs: ${rxscu%%.*} (RX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -h                     display this help and exit"
}
while getopts 'hu:p:v' c; do
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

echo -e "\nset up '${rxscu%%.*}'\n------------"
timeout 20 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_ALL"

# enable MPS task of rxscu
timeout 20 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && start_test4 \$DEV_RX"

echo "wait $sleep_sec seconds (start Xenabay schedule now)"
echo "------------"
sleep $sleep_sec  # wait for given seconds

# disable MPX task of rxscu"
timeout 20 sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && stop_test4 \$DEV_RX && \
    read_counters \$DEV_RX $verbose && result_ow_delay \$DEV_RX $verbose"
