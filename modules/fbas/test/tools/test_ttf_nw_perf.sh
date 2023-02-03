#!/bin/bash

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu_name="scuxl0497"
txscu_name="scuxl0396"
rxscu="scuxl0497.$domain"
txscu="scuxl0396.$domain"
fw_scu_def="fbas.scucontrol.bin"      # default LM32 FW for TX/RX SCUs
fw_scu_multi="fbas16.scucontrol.bin"  # supports up to 16 MPS channels

usage() {

    echo "Run a MPS signalling test between TX and RX SCUs."
    echo "User might be logged in to both SCUs to perform optional pre-check."
    echo
    echo "Usage: $0 [options]"
    echo
    echo " options:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpassd>         user password"
    echo "  -t <TX SCU>            transmitter SCU, by default $txscu_name"
    echo "  -r <RX SCU>            receiver SCU, by default $rxscu_name"
    echo "  -y                     'yes' to all prompts"
    echo "  -v                     verbosity for the measurement results"
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

setup_nodes() {
    echo -e "\n--- check deployment ---\n"

    filenames="$fw_scu_def $fw_scu_multi $script_rxscu"

    for filename in $filenames; do
        timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "if [ ! -f $filename ]; then echo $filename not found on ${rxscu}; exit 2; fi"
        result=$?
        report_check $result $filename $rxscu
    done

    echo -e "\nset up RX=$rxscu_name ...\n"
    unset sender_opts
    mac_txscu=$(timeout 10 sshpass -p "$userpasswd" ssh "$username@$txscu" "eb-mon -m dev/wbm0")
    if [ $? -ne 0 ]; then
        echo "Sender ID of $txscu_name: SENDER_ANY (device is not unaccessable)"
        sender_opts="SENDER_ANY"
    else
        echo "Sender ID of $txscu_name: $mac_txscu"
        sender_opts="SENDER_TX $mac_txscu"
    fi

    output=$(timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx $fw_scu_def $sender_opts")
    ret_code=$?
    if [ $ret_code -ne 0 ]; then
        echo "Error ($ret_code): cannot set up $rxscu_name"
        exit 1
    fi
    echo -e "\nset up TX=$txscu_name ...\n"
    output=$(timeout 10 sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && setup_mpstx")
    ret_code=$?
    if [ $ret_code -ne 0 ]; then
        echo "Error ($ret_code): cannot set up $txscu_name"
        exit 1
    fi
}

measure_nw_perf() {
    echo -e "\n--- enable MPS operation (RX, TX) ---\n"
    sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && enable_mps \$DEV_RX"
    sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && enable_mps \$DEV_TX"

    # start test
    sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && start_nw_perf"

    echo -e "\n--- disable MPS operation (TX, RX) ---\n"
    sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && disable_mps \$DEV_TX"
    sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && disable_mps \$DEV_RX"

    # report test result
    echo -e "\n--- report test result (TX, RX) ---\n"
    echo -n "TX: "
    sshpass -p "$userpasswd" ssh "$username@$txscu" \
        "source setup_local.sh && \
        read_counters \$DEV_TX $verbose && \
        result_tx_delay \$DEV_TX $verbose && \
        result_sg_latency \$DEV_TX $verbose"
    echo -n "RX: "
    sshpass -p "$userpasswd" ssh "$username@$rxscu" \
        "source setup_local.sh && \
        read_counters \$DEV_RX $verbose && \
        result_ow_delay \$DEV_RX $verbose && \
        result_ttl_ival \$DEV_RX $verbose"
}

measure_ttl() {
    echo -e "enable MPS operation of RX"
    sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && enable_mps \$DEV_RX"

    echo -e "toggle MPS operation of TX:\n"
    for i in $(seq 1 10); do
        echo -en "   $i : enable  "
        sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && enable_mps \$DEV_TX"
        sleep 1
        echo -en "   $i : disable "
        sshpass -p "$userpasswd" ssh "$username@$txscu" "source setup_local.sh && disable_mps \$DEV_TX"
        sleep 2
    done

    echo -e "disable MPS operation of RX"
    sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && disable_mps \$DEV_RX"

    echo -e "\n--- report TTL measurement ---\n"
    sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && result_ttl_ival \$DEV_RX \$addr_cnt1 $verbose"
}

unset username userpasswd option verbose
unset OPTIND

while getopts 'hyu:p:vt:r:' c; do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        y) option="auto" ;;
        v) verbose="yes" ;;
        t) txscu_name=$OPTARG; txscu=$txscu_name.$domain ;;
        r) rxscu_name=$OPTARG; rxscu=$rxscu_name.$domain ;;
        *) usage; exit 1 ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$rxscu_name, $txscu_name': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

echo -e "\n--- Step 1 - set up nodes (RX, TX)---"
setup_nodes

# optional pre-check before real test
echo -e "\n--- Step 2 - pre-check (RX, TX) ---\n"
pre_check

if [ "$option" != "auto" ]; then
    user_approval
fi

echo -e "\n--- Step 3 - measure network performance ---\n"
measure_nw_perf

# TTL measurement
echo -e "\n--- Step 4 - measure TTL ---\n"
measure_ttl
