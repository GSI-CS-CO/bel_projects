#!/bin/bash

# Used for the throughput measurement of the access layer. The measurement is based on the
# RFC2889 partial mesh 1:N test.

# XenaBay - perform RFC2889 partial mesh 1:16 test
# DM (pexaria28/32, dev/wbm{1/0} @tsl014) - generate DM broadcast with different rates

# Brief information about partial mesh 1:N is given below.
# It is used to determine the throughput, frame losses and forwarding rates of the switch
# when the stream is transmitted from 1-to-N or from N-to-1 port, measuring the capability
# of the switch device to forward frames without losses.

# source and init commons
script_dir=$(dirname "$(readlink -f "$0")")
if source "$script_dir/common.sh"; then
    echo "sourced $script_dir/common.sh"
else
    echo "failed to source $script_dir/common.sh. Exit!" >&2
    exit 1
fi

# initialize everything
setup_dirs
setup_infra
setup_sched_dir
setup_ssh_opts

# timing message rates that should be measured
dm_bc_rate=1000 # default timing msg rate [Hz]

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Generate DM broadcast with a specified data rate"
    echo "Used SCUs: $rxscu_name (RX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -r <message rate>      message rate for DM broadcast, msg/s"
    echo "  -s <DM schedule>       external file with DM schedule"
    echo "  -f <LM32 firmware>     firmware binary file"
    echo "  -h                     display this help and exit"
    echo
    echo "Example: $0 -r 1000 -s my_mps_basic_loop.dot -f $fw_rxscu"
}

unset username userpasswd sched_filename sched_filepath
unset OPTIND

while getopts 'hu:p:r:s:f:' c; do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        r) dm_bc_rate=$OPTARG ;;
        s) sched_filename=$OPTARG; sched_filepath="$sched_dir/$sched_filename" ;;
        f) fw_rxscu=$OPTARG ;;
        *) usage; exit 1 ;;
    esac
done

validate_sched_file "$sched_filepath"
validate_user username userpasswd      # pass variable references to validate and populate them

# setup everything
echo -e "\n--- 1. Set up DM=$datamaster ---\n"
setup_dm

echo -e "\n--- 2. Check deployment in TR=$rxscu_name ---\n"
check_tr

echo -e "\n--- 3. Set up TR=$rxscu_name ---\n"
get_sender_ids ids "$sched_filepath" # extract the sender IDs from a given file
if [[ -n "${ids[@]}" ]]; then
    setup_tr "${ids[@]}"
fi

# start measurements
echo -e "\n--- 4. Start the measurements ---\n"

unset results

t_period=$(( 1000000000 / $dm_bc_rate )) # period of tmg msgs block [ns]

header="$(printf "msg rate=%s Hz, period=%s ns" $dm_bc_rate $t_period)"
echo "Measurement: $header"

# reset the FW in receiver node
echo -en " reset eCPU (LM32) of '$rxscu_name': "
reset_tr_ecpu "${ids[@]}"

# enable MPS task of rxscu
echo -en " enable MPS operation of '$rxscu_name': "
enable_tr_mps

# start a schedule on DM
echo -en " start a schedule on '$datamaster': "
start_dm_schedule

read -p " press 'Enter' to stop the DM schedule ..."

# stop the running schedule on DM
echo -en " stop the running schedule on '$datamaster': "
stop_dm_schedule

# stop the operation of rxscu"
echo -en " stop operation of '$rxscu_name': "
stop_rx_operation

# obtain stats from TR
echo -en " obtain stats from '$rxscu_name': "
output=$(run_remote $rxscu \
    "source setup_local.sh && \
    read_counters \$rx_node_dev && \
    result_msg_delay \$rx_node_dev")
ret_code=$?
report_code $ret_code
exit_on_fail $ret_code

output=${output//$'\n'/ }          # replace all 'newline' with a space
output=$(echo $output | tr -s ' ') # remove consecutive spaces
arr=($output)                      # create an array

# format counters
lines+="RX msgs    : ${arr[1]}\n"    # 2nd element
lines+="Ovf msgs   : ${arr[2]}\n"    # 3rd element
lines+="Old msgs   : ${arr[3]}\n"    # 4th element
lines+="Bad msgs   : ${arr[4]}\n"    # 5th element

# format delays
lines+="--- delays, us (avg, min, max, valid, all)\n"
lines+="Msg delay  : ${arr[@]:5:5}\n" # get 5 elements starting at index 5

echo -e "\n$datamaster:$sched_filename $rxscu:$fw_rxscu host:$localhost ($(date))\n"
echo -e "$lines"
