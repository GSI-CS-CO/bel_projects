#!/bin/bash

# Test to determine the maximum data rate for receiver
# DM - pexaria28/32 (dev/wbm1, dev/wbm0) @tsl014
# RX SCU - scuxl0497

# [1] https://stackoverflow.com/questions/5799303/print-a-character-repeatedly-in-bash
# [2] Math arithmetic: how to do calculation in bash?, https://www.shell-tips.com/bash/math-arithmetic-calculation/#gsc.tab=0

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

get_tr_measurements() {

    # $1 - return/reply variable
    # $2 - target TR

    local -n ret="$1"
    shift

    local output
    output=$(run_remote $1 \
    "source setup_local.sh && \
    read_counters \$rx_node_dev && \
    result_rx_delay \$rx_node_dev && \
    result_msg_delay \$rx_node_dev && \
    result_ttl_ival \$rx_node_dev && \
    result_ml_period \$rx_node_dev && \
    result_eca_delay \$rx_node_dev && \
    read_array \$rx_node_dev 8")
    ret_code=$?
    exit_on_fail $ret_code

    output=${output//$'\n'/ }          # replace all 'newline' with a space
    output=$(echo $output | tr -s ' ') # remove consecutive spaces

    ret="$output"
}

format_measurements() {

    # $1 - return/reply variable
    # $@ - array with measurement values (in string)

    local -n ret="$1"
    shift

    local output=($@) # make string to array
    local line

    # counters
    line+="\tRX msgs    : ${output[1]}\n"    # 2nd element
    line+="\tOvf msgs   : ${output[2]}\n"    # 3rd element
    line+="\tOld msgs   : ${output[3]}\n"    # 4th element
    line+="\tBad msgs   : ${output[4]}\n"    # 5th element

    # delays
    line+="\t--- delays, us (avg, min, max, valid, all)\n"
    line+="\tECA delay  : ${output[@]:25:5}\n" # get 5 elements starting at index 25
    line+="\tRX delay   : ${output[@]:5:5}\n"  # get 5 elements starting at index 5
    line+="\tMsg delay  : ${output[@]:10:5}\n" # get 5 elements starting at index 10
    line+="\tTTL period : ${output[@]:15:5}\n" # get 5 elements starting at index 15
    line+="\tLoop period: ${output[@]:20:5}\n" # get 5 elements starting at index 20
    line+="\t--- action handling rate\n"
    line+="\t${output[@]:30:8}\n"              # get 8 elements starting at index 30

    ret="$line"
}

is_measurement_failed() {

    # $1 - return/reply variable
    # $@ - string with measurements

    # measurements[1] - eca valid, expected non-zero count
    # measurements[2] - eca overflow, expected zero count
    # measurements[8] - average messaging delay, expected below 1 ms

    local -n ret="$1"
    shift
    local output=($@)  # make array
    local failed=0 # false

    eca_valid=${output[1]}
    eca_valid=$(( 10#$eca_valid ))  # convert a string to integer

    eca_overflow=${output[2]}
    eca_overflow=$(( 10#$eca_overflow ))

    # failure: if the 'ECA overflow' counter has non-zero or
    # 'ECA valid' counter has zero value
    if [ $eca_overflow -ne 0 ] || [ $eca_valid -eq 0 ]; then
        failed=1 # true
    fi

    # failure: if the 'average messaging delay' is higher than 1 ms
    avg_msg_dly=${output[8]}
    avg_msg_dly=$(( 10#$avg_msg_dly ))     # convert a string to integer
    if [ $avg_msg_dly -gt 1000000 ]; then
        failed=2 # true
    fi

    ret=$failed
}

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Test to determine the maximum data rate for receiver"
    echo "Used SCUs: $rxscu_name (RX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -s <DM schedule>       external file with DM schedule"
    echo "  -f <LM32 firmware>     firmware binary file"
    echo "  -m                     enable higher message rates (> 10KHz)"
    echo "  -h                     display this help and exit"
    echo
    echo "Example: $0 -s my_mps_rx_rate_1.dot -f $fw_rxscu"
}

unset username userpasswd sched_filename enable_high_rates
unset OPTIND

while getopts 'hu:p:s:f:m' c; do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        s) sched_filename=$OPTARG; sched_filepath="$sched_dir/$sched_filename" ;;
        f) fw_rxscu=$OPTARG ;;
        m) enable_high_rates="y" ;;
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

echo -e "\n--- 3. Check DM schedule in '$sched_filename' ---\n"
m_rates=()
check_dm_schedule "$sched_filename" "$enable_high_rates" m_rates m_block

echo -e "\n--- 4. Set up TR=$rxscu_name ---\n"
get_sender_ids ids "$sched_filepath" # extract the sender IDs from a given file
if [[ -n "${ids[@]}" ]]; then
    setup_tr "${ids[@]}"
fi

# start measurements
echo -e "\n--- 5. Start the measurements ---\n"

unset results
for i in ${!m_rates[@]}; do

    rate=${m_rates[$i]}
    t_period=$(bc <<< "1000000000 / $rate * $m_block") # period of tmg msgs block [ns]

    header="$(printf "%6s | msg rate=%s Hz, period=%s ns" $i $rate $t_period)"
    echo "Measurement: $header"
    dashed_line="$(printf "%0.s-" $(seq 1 ${#header}))" # print a given number of '-'
    results+="$header\n"
    results+="$dashed_line\n"

    # reset the FW in receiver node
    echo -en " reset eCPU (LM32) of '$rxscu_name': "
    reset_tr_ecpu "${ids[@]}"

    # enable MPS task of rxscu
    echo -en " enable MPS operation of '$rxscu_name': "
    enable_tr_mps

    # start a schedule on DM
    echo -en " start a schedule on '$datamaster': "
    run_finite_dm_schedule

    # disable MPX task of rxscu"
    echo -en " disable MPS operation of '$rxscu_name': "
    disable_tr_mps

    # obtain TR measurements
    echo -en " obtain measurements from '$rxscu_name': "
    get_tr_measurements measures $rxscu
    [[ -z "$measures" ]] && echo "FAIL" || echo "OK"

    format_measurements new_line "$measures"
    results+="$new_line\n"
    results+="$dashed_line\n"

    # break loop if the measurement fails (ECA overflow, messaging delay > 1ms)
    is_measurement_failed break_loop "$measures"
    [[ $break_loop -ne 0 ]] && break

done

echo -e "\n$datamaster:$sched_filename $rxscu:$fw_rxscu host:$localhost ($(date))\n"
echo -e "$results"
