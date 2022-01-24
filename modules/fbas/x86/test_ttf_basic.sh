#!/bin/bash

# Test procedure to check timing message transfer between 2 SCUs:
# - RX SCU
# - TX SCU

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
txscu="scuxl0396.$domain"
sleep_sec=10

# get username and password to access SCUs
read -rp "username to access '${rxscu%%.*}, ${txscu%%.*}': " username
read -rsp "password for '$username' : " userpasswd

echo -e "\nset up nodes\n------------"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && setup_mpstx"

echo "start test4 (RX, TX)"
echo "-----------"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && start_test4 \$DEV_RX"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && start_test4 \$DEV_TX"

echo "wait $sleep_sec seconds"
echo "------------"
sleep $sleep_sec  # wait for given seconds

echo "stop test4 (TX, RX)"
echo "----------"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && stop_test4 \$DEV_TX"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && stop_test4 \$DEV_RX"
