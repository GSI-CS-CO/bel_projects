#!/bin/bash

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu="scuxl0411.$domain"

fw_scu_def="fbas.scucontrol.bin"      # default LM32 FW for TX/RX SCUs
fw_scu_multi="fbas16.scucontrol.bin"  # supports up to 16 MPS channels
fn_mps_events="simple_mps_events.sched" # filename with schedule for the MPS events
n_iterations=1                        # number of iterations of the schedule

usage() {

    echo "Usage: $0 [OPTION]"
    echo "Count locally injected MPS events (by saft-dm)"
    echo
    echo "Used SCUs: ${rxscu%%.*} (RX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpassd>         user password"
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

setup_node() {
    echo -e "\n--- check deployment ---\n"

    filenames="$fw_scu_def $fw_scu_multi $script_rxscu $fn_mps_events"

    for filename in $filenames; do
        timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "if [ ! -f $filename ]; then echo $filename not found on ${rxscu}; exit 2; fi"
        result=$?
        report_check $result $filename $rxscu
    done

    echo -e "\n--- setup RX node ---\n"
    timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx $fw_scu_multi SENDER_TX"
    if [ $? -ne 0 ]; then
        echo "Error: cannot set up ${rxscu%%.*} -> timed out"
        exit 1
    fi
}

inject_events() {
    # $1 - number of iterations of the given schedule
    # $2 - filename with schedule for the MPS events

    echo -e "\n--- enable MPS operation (RX) ---\n"
    sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && enable_mps \$DEV_RX"

    # start local injection of MPS events
    sshpass -p "$userpasswd" ssh $username@$rxscu "source setup_local.sh && inject_mps_events $1 $2"

    echo -e "\n--- disable MPS operation (RX) ---\n"
    sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && disable_mps \$DEV_RX"
}

show_rx_stats() {
    # report test result
    echo -e "\n--- statistics (RX) ---\n"
    echo -n "RX: "
    sshpass -p "$userpasswd" ssh "$username@$rxscu" \
        "source setup_local.sh && \
        read_counters \$DEV_RX $verbose && \
        result_ow_delay \$DEV_RX $verbose && \
        result_ttl_ival \$DEV_RX $verbose"
}

unset username userpasswd option verbose
unset OPTIND

while getopts 'hyu:p:v' c; do
    case $c in
        h) usage; exit 1 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        y) option="auto" ;;
        v) verbose="yes" ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '${rxscu%%.*}': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

# set up RX node
echo -e "\n--- Step 1 - set up node (RX)---"
setup_node

# inject MPS events locally
echo -e "\n--- Step 2 - inject MPS events ---"
inject_events $n_iterations $fn_mps_events

# show the statistics of RX node
echo -e "\n--- Step 3 - show RX stats ---"
show_rx_stats
