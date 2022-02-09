#!/bin/bash

domain=$(hostname -d)
rxscu="scuxl0497.$domain"
txscu="scuxl0396.$domain"
unset username userpasswd option

usage() {

    echo "Usage: $0 [OPTION]"
    echo "Run a MPS signalling test between TX and RX SCUs."
    echo "User might be logged in to both SCUs to perform optional pre-check."
    echo "Used SCUs: ${rxscu%%.*} (RX), ${txscu%%.*} (TX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpassd>         user password"
    echo "  -y                     'yes' to all prompts"
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

    echo "(RX): drive B1 output:"
    echo "      saft-io-ctl tr0 -n B1 -o 1 -d 1"
    echo "      saft-io-ctl tr0 -n B1 -o 1 -d 0"
}

while getopts 'hyu:p:' c; do
    case $c in
        h) usage; exit 1 ;;
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

echo -e "\n--- Step 1 - set up nodes (RX, TX)---"
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

if [ "$option" != "auto" ]; then
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
sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && result_nw_perf \$DEV_RX \$addr_cnt1 && result_ow_delay \$DEV_RX \$addr_cnt1"
