#!/bin/bash

# Test the ECA to SCU bus slave transfer: high 16 bits of upper 32 bits in parameter of
# the received timing messages are transferred to the echo register of the DIOB card,
# installed in the RX SCU.
# DM - pexaria28/32 (dev/wbm1, dev/wbm0) @tsl014
# RX SCU - scuxl0497

# Derived from 'measure_ttf_rx_rate.sh'
# Run: ./test_eca_scu_bus_slave.sh -u root -s my_mps_eca_scu_bus_slave_16.dot -f fbas16.scucontrol.bin

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
    stats_eca_tap \$rx_node_dev")
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
    line+="\tRX msgs    : ${output[0]}\n"    # 1st element
    line+="\tLate msgs  : ${output[1]}\n"    # 2nd element
    line+="\tLatency    : ${output[2]}\n"    # 3rd element

    ret="$line"
}

is_measurement_failed() {

    # $1 - return/reply variable
    # $@ - string with measurements

    # measurements[0] - RX msg count, expected non-zero count
    # measurements[1] - RX late msg count, expected non-zero count

    local -n ret="$1"
    shift
    local output=($@)  # make array
    local failed=0 # false

    n_msg=${output[0]}
    n_msg=$(( 10#$n_msg ))  # convert a string to integer

    n_late_msg=${output[1]}
    n_late_msg=$(( 10#$n_late_msg ))

    # failure: if the 'n_msg' or 'n_late_msg' count has zero value
    if [ $n_msg -eq 0 ] || [ $n_late_msg -eq 0 ]; then
        failed=1 # true
    fi

    ret=$failed
}

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Test the ECA to SCU bus slave transfer"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -s <DM schedule>       external file with DM schedule"
    echo "  -f <LM32 firmware>     firmware binary file"
    echo "  -r <target_rxscu>      target RX SCU (by default $rxscu_name is chosen)"
    echo "  -m                     enable higher messagging rates (up to 50KHz)"
    echo "  -h                     display this help and exit"
    echo
    echo "Example: $0 -s my_mps_rx_rate_1.dot -f $fw_rxscu"
}

unset username userpasswd sched_filename target_rxscu enable_high_rates
unset OPTIND

while getopts 'hu:p:r:s:f:m' c; do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        r) target_rxscu=$OPTARG ;;
        s) sched_filename=$OPTARG; sched_filepath="$sched_dir/$sched_filename" ;;
        f) fw_rxscu=$OPTARG ;;
        m) enable_high_rates="y" ;;
        *) usage; exit 1 ;;
    esac
done

if [[ -n "$target_rxscu" ]]; then
    rxscu_name="$target_rxscu"
fi

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

    # stop the operation of rxscu"
    echo -en " stop operation of '$rxscu_name': "
    stop_rx_operation

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
