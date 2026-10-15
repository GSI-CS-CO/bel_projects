#!/bin/bash

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)
rxscu_name="scuxl0497"
def_txscu_name="scuxl0396"
txscu_name=()                         # array with transmitter names
rxscu="$rxscu_name.$domain"
txscu=()                              # array with transmitter domain names
fw_scu_def="fbas16.scucontrol.bin"    # FW that supports up to 16 TX nodes, each has 1 MPS channels
ssh_opts="-o StrictHostKeyChecking=no"   # no hostkey checking
getopt_opts="u:p:t:r:g:i:ceyvh"       # user options
n_tx_rates=21                         # number of TX messaging rates
msg_rates=(33333 100000 50000 20000 10000 5000 3333 2500 \
            # 30,  10,    20,   50,   100,  200, 300, 400 [Hz]
            2000   1250 1000  833   666   555 \
            # 500, 800, 1K,   1.2K, 1.5K, 1.8K [Hz]
            500    333  250   200   166   143  125)
            # 2K,  3K,  4K,   5K,   6K,   7K,  8K [Hz]

pids=()                               # sub-process ids for each running test on SCUs

# signal handler
sigint_handler()
{
    # stop all TX and RX devices, if CTRL+C is requested
    echo

    local pcnt=0

    for pid in ${pids[@]}; do
        kill $pid
        pcnt=$(( $pcnt + 1 ))
    done

    if [ $pcnt -ne 0 ]; then
        echo "Stopped $pcnt sub-process(es)."
        pcnt=0
    fi

    for i in ${!txscu[@]}; do
        run_remote ${txscu[$i]} "source setup_local.sh && eb-reset \$tx_node_dev cpureset 0"
    done

    run_remote $rxscu "source setup_local.sh && eb-reset \$rx_node_dev cpureset 0"

    echo "All LM32 CPUs are resetted. Exit."
    exit
}

usage() {

    echo "Run a MPS signalling test between TX and RX SCUs."
    echo
    echo "Usage: $0 [options]"
    echo
    echo " options:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpassd>         user password"
    echo "  -t <TX SCU>            transmitter SCU, by default $def_txscu_name"
    echo "  -r <RX SCU>            receiver SCU, by default $rxscu_name"
    echo "  -g <event gen period>  pseudo event generation period (10 seconds by default)"
    echo "  -i <index of TX rate>  index of the TX messaging rate (by default i=0 or 30Hz)"
    echo "                         0=30, 1=10, 2=20 3=50Hz, 4=100, 5=200, 6=300, 7=400 [Hz]"
    echo "                         8=500, 9=800, 10=1K, 11=1.2K, 12=1.5K, 13=1.8K [Hz]"
    echo "                         14=2K, 15=3K, 16=4K, 17=5K, 18=6K, 19=7K, 20=8K [Hz]"
    echo "  -c                     optional, maximum RX message rate check (assert max_rx_rate < 1,25 * tx_rate, us)"
    echo "  -e                     exclude TTL measurement"
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

info_messaging_rate() {
    # $1 - index

    if [ -n "$1" ]; then
        msg_rate_us=${msg_rates[$1]}
        msg_rate_hz=$(bc <<< "scale=1; 1000000/$msg_rate_us")

        printf "TX: msg rate = %.1f Hz (%d us)\n" $msg_rate_hz $msg_rate_us
    fi
}

setup_nodes() {

    filenames="$fw_scu_def $script_rxscu"

    mac_txscu=()
    all_scu=(${txscu[@]} "$rxscu")

    for scu in "${all_scu[@]}"; do
        # get MAC address
        mac_scu=$(run_remote $scu "eb-mon -m dev/wbm0")
        ret_code=$?
        if [ $ret_code -eq 0 ]; then
            if [ "$scu" != "$rxscu" ]; then
                mac_txscu+=($mac_scu)
                echo "TX=$scu: $mac_scu"
            else
                echo "RX=$scu: $mac_scu"
            fi
        fi

        # check deployment
        check_deployment $scu $filenames

        # set up TX nodes
        if [ "$scu" != "$rxscu" ]; then
            output=$(run_remote $scu "source setup_local.sh && setup_mpstx $idx_msg_period")
            ret_code=$?
            if [ $ret_code -ne 0 ]; then
                echo "Error ($ret_code): cannot set up $scu"
                exit 1
            fi
        fi
        echo
    done

    # set up RX node
    sender_opts="SENDER_ANY"
    if [ ${#mac_txscu[@]} -eq 0 ]; then
        sender_opts="SENDER_ANY"
    else
        sender_opts="SENDER_TX ${mac_txscu[@]}"
    fi

    output=$(run_remote $rxscu "source setup_local.sh && setup_mpsrx $fw_scu_def $sender_opts")
    ret_code=$?
    if [ $ret_code -ne 0 ]; then
        echo "Error ($ret_code): cannot set up $rxscu_name"
        exit 1
    fi

    echo "Sender ID(s): ${mac_txscu[@]}"
}

measure_nw_perf() {

    # use nameref to return a value to the given input parameter
    local -n max_rx_rate="$1"    # RX msg rate
    # $2 - optional counter for measurement
    local measure_cnt=$2

    echo -e "start the measurement: $measure_cnt\n"

    info_messaging_rate $idx_msg_period

    output=$(run_remote $rxscu "source setup_local.sh && start_operation \$rx_node_dev")

    # use local script to print info
    output=$(source $dir_name/../scu/setup_local.sh && info_nw_perf $gen_period)
    echo -e "$output\n"

    # enable simultaneous operation of TX nodes
    for i in ${!txscu[@]}; do
        echo ${txscu[$i]}

        # enable MPS operation, start test => keep process ID
        output=$(run_remote ${txscu[$i]} "source setup_local.sh && start_operation \$tx_node_dev")

        # start test sub-process and keep its process ID
        run_remote ${txscu[$i]} "source setup_local.sh && start_nw_perf $gen_period" &
        pids[$i]=$!
    done

    # notice the start time (after calling sub-process in the last device)
    start=$(date +%s)

    # wait until all sub-processes are complete
    for pid in ${pids[@]}; do
        wait $pid
    done

    # notice the end time (sub-process is finished in the last device)
    end=$(date +%s)
    # calculate the runtime
    runtime=$((end - start))

    echo -e "stop the measurements, runtime $runtime seconds\n"
    for scu in ${txscu[@]}; do
        output=$(run_remote $scu "source setup_local.sh && stop_operation \$tx_node_dev")
    done

    output=$(run_remote $rxscu "source setup_local.sh && stop_operation \$rx_node_dev")

    # report test result
    echo "measurement stats of MPS signaling: $measure_cnt"
    if [ -z "$verbose" ]; then
        echo -e "MPS node:       tx_cnt rx_vld rx_ovf\n"
    else
        echo
    fi

    sum_tx_cnt=0
    for scu in ${txscu[@]}; do
        cnt=$(run_remote $scu \
            "source setup_local.sh && \
            read_counters \$tx_node_dev $verbose")
        echo "TX (${scu%%.*}): $cnt"
	tx_cnt=${cnt%% *} # remove the right part including all spaces
	sum_tx_cnt=$(( $sum_tx_cnt + $tx_cnt ))
    done

    cnt=$(run_remote $rxscu \
        "source setup_local.sh && \
        read_counters \$rx_node_dev $verbose")
    echo "RX (${rxscu%%.*}): $cnt"

    rx_cnt=${cnt#* }     # remove the left part including first occurence of space
    rx_cnt=${rx_cnt%% *} # remove the right part including all spaces

    result="received $rx_cnt of $sum_tx_cnt"
    if [ $rx_cnt -eq $sum_tx_cnt ]; then
	echo -e "PASS: $result\n"
    else
	echo -e "FAIL: $result\n"
    fi

    # print measurement header
    if [ -z "$verbose" ]; then
        echo -e "Delay:  avg min max [us] vld all [])\n"
        # declare measurement entries
        tx_delay_entries=("eca dly" "tx dly " "tx rate" "ml prd ")
        rx_delay_entries=("eca dly" "rx dly " "msg dly" "rx rate" "mlp prd" "dio dly" "ttl ivl" "eca tap")
    fi

    i=0
    for scu in ${txscu[@]}; do
        echo "TX (${scu%%.*}):"

        # read command output line by line
        run_remote $scu \
            "source setup_local.sh && \
            result_eca_delay \$tx_node_dev $verbose && \
            result_tx_delay \$tx_node_dev $verbose && \
            result_msg_rate \$tx_node_dev $verbose && \
            result_ml_period \$tx_node_dev $verbose" |
        while IFS= read -r line; do
            delay_entry="${tx_delay_entries[$i]}"
            if [ -n "$delay_entry" ]; then
                delay_entry=" $delay_entry:"
            fi
            echo "$delay_entry $line"
            i=$((i + 1))
        done
    done

    i=0
    echo "RX (${rxscu%%.*}):"
    run_remote $rxscu \
        "source setup_local.sh && \
        result_eca_delay \$rx_node_dev $verbose && \
        result_rx_delay \$rx_node_dev $verbose && \
        result_msg_delay \$rx_node_dev $verbose && \
        result_msg_rate \$rx_node_dev $verbose && \
        result_ml_period \$rx_node_dev $verbose && \
        result_diob_delay \$rx_node_dev $verbose && \
        result_ttl_ival \$rx_node_dev $verbose" |
    while IFS= read -r line; do
        delay_entry="${rx_delay_entries[$i]}"
        if [ -n "$delay_entry" ]; then
            delay_entry=" $delay_entry:"
        fi
        echo "$delay_entry $line"
        i=$((i +  1))
    done

    run_remote $rxscu \
        "source setup_local.sh && \
        stats_eca_tap \$rx_node_dev $verbose"

    msg_rate=$(run_remote $rxscu "source setup_local.sh && result_msg_rate \$rx_node_dev")
    array_msg_rate=($msg_rate) # convert to array
    max_rx_rate="${array_msg_rate[2]}" # return the RX msg rate
}

measure_ttl() {
    echo -e "RX: start the FW operation\n"
    output=$(run_remote $rxscu "source setup_local.sh && start_operation \$rx_node_dev")

    echo -e "TX: start the FW operation: TX=${txscu_name[*]}"
    for scu in ${txscu[@]}; do
        output=$(run_remote $scu "source setup_local.sh && start_operation \$tx_node_dev")
    done

    echo -e "start the measurement\n"

    n_toggle=10
    echo -e "toggle MPS operation (n=$n_toggle): TX=${txscu_name[*]}"
    for i in $(seq 1 $n_toggle); do
        echo -en " $i: enable \r"

        for scu in ${txscu[@]}; do
            output=$(run_remote $scu "source setup_local.sh && start_operation \$tx_node_dev")
        done

        sleep 1
        echo -en " $i: disable\r"

        for scu in ${txscu[@]}; do
            output=$(run_remote $scu "source setup_local.sh && stop_operation \$tx_node_dev")
        done

        sleep 1
    done

    echo -e "\nstop the measurement\n"
    output=$(run_remote $rxscu "source setup_local.sh && stop_operation \$rx_node_dev")

    echo -e "measurement stats: TTL\n"
    run_remote $rxscu "source setup_local.sh && result_ttl_ival \$rx_node_dev \$addr_cnt1 $verbose"
}

trap sigint_handler SIGINT

unset username userpasswd gen_period idx_msg_period rx_msg_rate_check exclude_ttl auto verbose
unset OPTIND

while getopts $getopt_opts c; do
    case $c in
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        t) txscu_name+=("$OPTARG"); txscu+=("$OPTARG.$domain") ;;
        r) rxscu_name=$OPTARG; rxscu=$OPTARG.$domain ;;
        g) gen_period=$OPTARG ;;
        i) idx_msg_period=$OPTARG ;;
        c) rx_msg_rate_check="yes" ;;
        e) exclude_ttl="exclude_ttl" ;;
        y) auto="auto" ;;
        v) verbose="yes" ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done

# check the index of the TX messaging period
if [ "$idx_msg_period" ]; then
    num=$(($idx_msg_period)) 2>/dev/null
    if [ $num -ge $n_tx_rates ]; then
        echo "Error: invalid index for TX messaging period: $num (valid index is less than $n_tx_rates). Exit!"
        usage; exit 1
    fi
fi

# get the default transmitter SCU name
if [ ${#txscu_name[@]} -eq 0 ]; then
    txscu_name+=("$def_txscu_name")
    txscu+=("$def_txscu_name.$domain")
fi

scu_names="$rxscu_name, ${txscu_name[*]}"
# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$scu_names': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username@{$scu_names}': " userpasswd; echo
fi

# set the pseudo event generation period
if [ -z "$gen_period" ]; then
    gen_period=10
fi

echo -e "\n--- Step 1: set up nodes (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"
setup_nodes

echo -e "\n--- Step 2: measure network performance (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"

measure_cnt=1 # measurement counter

# optional check for max RX msg rate
max_rx_msg_rate=${msg_rates[$idx_msg_period]}
rx_rate_threshold=$(( max_rx_msg_rate + max_rx_msg_rate / 4 ))

while [ $max_rx_msg_rate -lt $rx_rate_threshold ]; do # normal/expected behaviour
    measure_nw_perf max_rx_msg_rate  $measure_cnt     # get the max RX msg rate
    max_rx_msg_rate=$(( 10#$max_rx_msg_rate ))        # convert a string to integer

    if [ -z "$rx_msg_rate_check" ]; then
        break
    fi

    measure_cnt=$(( measure_cnt + 1 ))
done

if [ $max_rx_msg_rate -ge $rx_rate_threshold ]; then
    echo "Got high msg rate than expected [us]: $max_rx_msg_rate >> ${msg_rates[$idx_msg_period]}"
fi

if [ -n "$exclude_ttl" ]; then
    exit 0
fi

# TTL measurement
echo -e "\n--- Step 3: measure TTL (RX=$rxscu_name, TX=${txscu_name[*]}) ---\n"
measure_ttl
