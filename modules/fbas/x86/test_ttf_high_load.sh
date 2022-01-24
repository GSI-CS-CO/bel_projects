#!/bin/bash

# Test procedure dedicated for the Xenabay 'high_load' testbed
# SCU (scuxl0497) operates as RX node

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
sleep_sec=30

# get username and password to access SCUs
read -rp "username to access '${rxscu%%.*}': " username
read -rsp "password for '$username' : " userpasswd

echo -e "\nset up '${rxscu%%.*}'\n------------"
timeout 20 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx"

# enable MPS task of rxscu
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && start_test4 \$DEV_RX"

echo "wait $sleep_sec seconds"
echo "------------"
sleep $sleep_sec  # wait for given seconds

# disable MPX task of rxscu"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && stop_test4 \$DEV_RX"
