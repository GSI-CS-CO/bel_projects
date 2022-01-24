#!/bin/bash

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
txscu="scuxl0396.$domain"

usage() {

    echo -e "\nThis is a MPS signalling test between TX and RX SCUs."
    echo -e "User might be logged in to both SCUs to perform optional pre-check."
    echo -e "Used SCUs: ${rxscu%%.*} (RX), ${txscu%%.*} (TX)\n"
    echo -e "Usage: $0 [auto]"
    echo -e "       auto - user input is taken from environment variables"
    echo -e "              GSI_SCU_ROOT_USER, GSI_SCU_ROOT_PASSWD\n"
}

user_approval() {
    echo -en "\nCONITNUE (Y/n)? "
    read -r answer

    if [ "$answer" != "y" ] && [ "$answer" != "Y" ] && [ "$answer" != "" ]; then
        exit 1
    fi
}

pre_check() {
    echo "TX: snoop TLU event (for IO action):"
    echo "    saft-ctl tr0 -xv snoop 0 0 0"
    echo "TX: events expected, when B1 output is driven on RX:"
    echo "    GID: 0x0fff EVTNO: 0x0100 Other: 0x000000001"
    echo "    GID: 0x0fff EVTNO: 0x0100 Other: 0x000000000"

    echo "(RX): drive B1 output:"
    echo "      saft-io-ctl tr0 -n B1 -o 1 -d 1"
    echo "      saft-io-ctl tr0 -n B1 -o 1 -d 0"
}

# print usage info
usage

# get username and password to access SCUs
if [ "$1" == "auto" ]; then
    username=$GSI_SCU_ROOT_USER
    userpasswd=$GSI_SCU_ROOT_PASSWD
else
    read -rp "username to access '${rxscu%%.*}, ${txscu%%.*}': " username
    read -rsp "password for '$username' : " userpasswd
fi

echo -e "\n--- Step 1 - set up nodes (RX, TX)---\n"
echo -e "\n--- setup RX node ---\n"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx"
if [ $? -ne 0 ]; then
    echo "Error: cannot set up ${rxscu%%.*} -> timed out"
    exit 1
fi
echo -e "\n--- setup TX node ---\n"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && setup_mpstx"
if [ $? -ne 0 ]; then
    echo "Error: cannot set up ${txscu%%.*} -> timed out"
    exit 1
fi

# optional pre-check before real test
echo -e "\n--- Step 2 - pre-check (RX, TX) ---\n"

pre_check

if [ "$1" != "auto" ]; then
    user_approval
fi

echo -e "\n--- Step 3 - enable MPS operation (RX, TX) ---\n"
sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && enable_mps \$DEV_RX"
sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && enable_mps \$DEV_TX"

# start test
sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && start_nw_perf"

echo -e "\n--- Step 4 - disable MPS operation (TX, RX) ---\n"
sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && disable_mps \$DEV_TX"
sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && disable_mps \$DEV_RX"

# report test result
echo -e "\n--- Step 5 - report test result (TX, RX) ---\n"
echo -n "TX "
sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && result_nw_perf \$DEV_TX \$addr_cnt1"
echo -n "RX "
sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && result_nw_perf \$DEV_RX \$addr_cnt1"

