#!/bin/bash

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu_name="scuxl0411"
rxscu="$rxscu_name.$domain"

fw_scu_def="fbas16.scucontrol.bin"    # default FW that supports up to 16 TX nodes, each has 1 MPS channel
fw_scu_multi="fbas16.scucontrol.bin"
fn_mps_events="simple_mps_events.sched" # filename with schedule for the MPS events
n_repeat=1                            # number of repeatations of the schedule

usage() {

    echo "Usage: $0 [OPTION]"
    echo "Count locally injected MPS events (by saft-dm)"
    echo
    echo "OPTION:"
    echo "  -s <SCU>               RX SCU, by default $rxscu_name"
    echo "  -u <username>          user name to log in to SCU"
    echo "  -p <userpassd>         user password"
    echo "  -n <number>            repeat the schedule, by default 1"
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
    echo -e "\n check deployment\n"

    filenames="$fw_scu_def $fw_scu_multi $script_rxscu $fn_mps_events"

    for filename in $filenames; do
        output=$(timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu \
            "if [ ! -f $filename ]; then echo $filename not found on ${rxscu}; exit 2; fi")
        ret_code=$?
        report_check $ret_code $filename $rxscu
    done

    echo -e "\n load FW ($fw_scu_multi) & configure\n"

    unset sender_opts
    mac_rxscu=$(timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" \
        "eb-mon -m dev/wbm0")
    ret_code=$?
    if [ $ret_code -ne 0 ]; then
        echo "FAIL ($ret_code): sender ID of $rxscu is unknown. Exit!"
        exit 1
    fi
    output=$(timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" \
        "source setup_local.sh && \
        setup_mpsrx $fw_scu_multi SENDER_TX $mac_rxscu")
    ret_code=$?
    if [ $ret_code -ne 0 ]; then
        echo "FAIL ($ret_code): cannot set up $rxscu_name. Exit!"
        exit 1
    fi
}

inject_events() {
    # $1 - number of iterations of the given schedule
    # $2 - filename with schedule for the MPS events

    echo -e "\n enable MPS operation (RX=$rxscu_name)"
    output=$(sshpass -p "$userpasswd" ssh $username@$rxscu \
        "source setup_local.sh && enable_mps \$rx_node_dev")

    # start local injection of MPS events
    echo -e " inject MPS events\n"
    sshpass -p "$userpasswd" ssh $username@$rxscu \
        "source setup_local.sh && inject_mps_events $1 $2"

    echo -e "\n disable MPS operation (RX=$rxscu_name)"
    output=$(sshpass -p "$userpasswd" ssh "$username@$rxscu" \
        "source setup_local.sh && disable_mps \$rx_node_dev")
}

show_rx_stats() {
    # report test result
    echo -en "\nRX: "
    sshpass -p "$userpasswd" ssh "$username@$rxscu" \
        "source setup_local.sh && \
        read_counters \$rx_node_dev $verbose && \
        result_msg_delay \$rx_node_dev $verbose && \
        result_ttl_ival \$rx_node_dev $verbose"
}

unset username userpasswd verbose
unset OPTIND

while getopts 'hys:u:p:vn:' c; do
    case $c in
        h) usage; exit 1 ;;
        s) rxscu_name=$OPTARG; rxscu="$rxscu_name.$domain" ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        v) verbose="yes" ;;
        n) n_repeat=$OPTARG ;;
        *) ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$rxscu_name': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

# set up RX node
echo -e "\n--- 1. Set up node (RX=$rxscu_name)---"
setup_node

# inject MPS events locally
echo -e "\n--- 2. Inject MPS events locally (repeat=$n_repeat, file=$fn_mps_events) ---"
inject_events $n_repeat $fn_mps_events

# show the statistics of RX node
echo -e "\n--- 3. Show statistics (RX=$rxscu_name) ---"
show_rx_stats
