#!/bin/bash

# Control procedure dedicated for the Xenabay 'high_load' testbed
# SCU (scuxl0497) operates as RX node

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
sleep_sec=30

unset username userpasswd

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
while getopts 'hu:p:' c; do
    case $c in
        h) usage; exit 1 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
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
timeout 20 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx"

# enable MPS task of rxscu
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && start_test4 \$DEV_RX"

echo "wait $sleep_sec seconds"
echo "------------"
sleep $sleep_sec  # wait for given seconds

# disable MPX task of rxscu"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && stop_test4 \$DEV_RX"
