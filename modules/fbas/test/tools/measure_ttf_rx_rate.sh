#!/bin/bash

# Test to determine the maximum data rate for receiver
# DM - pexaria28/32 (dev/wbm1, dev/wbm0) @tsl014
# RX SCU - scuxl0497

# [1] https://stackoverflow.com/questions/5799303/print-a-character-repeatedly-in-bash
# [2] Math arithmetic: how to do calculation in bash?, https://www.shell-tips.com/bash/math-arithmetic-calculation/#gsc.tab=0

abs_path=$(readlink -f "$0")
dir_name=${abs_path%/*}
source $dir_name/test_ttf_basic.sh -s  # source the specified script

domain=$(hostname -d)             # domain name of local host
rxscu_name="scuxl0497"            # name of RX SCU
rxscu="$rxscu_name.$domain"       # full name of RX SCU, name=${rxscu%%.*}
datamaster="tsl014"               # Data Master
login_dm="root@$datamaster"       # pubkey login (alias 'backdoor') is used for login
mngmasters=( tsl101 )             # Management Masters
localhost=$(hostname -s)          # local host

fw_rxscu="fbas128.scucontrol.bin" # default LM32 FW for RX SCU

sched_dir="${dir_name%/*}/dm"     # directory with DM schedules

# determine if a local host is a management master (alias tslhost)
unset tslhost
for mm in "${mngmasters[@]}"; do
    if [ "$localhost" == "$mm" ]; then
        tslhost="$mm"
        break
    fi
done

# for non-tslhosts, locate a proper DM schedule path
if [ -z "$tslhost" ]; then
    sched_dir="${PWD/fbas*/fbas}/test/dm"
fi

dst_test_dir="fbas_test"          # destination directory for DM scripts
src_test_dir="${dir_name%/*}"     # source test directory

ssh_opts="-o StrictHostKeyChecking=no"
scp_opts="-r"                     # -r: recursive copy

# Check if scp supports the '-O' option (use the legacy SCP protocol), required to access hosts, SCUs with older SSH
if scp -O $0 /dev/null &>/dev/null; then
    scp_opts+=" -O"
fi

# | period    | msg rate  | data rate |vld eca|ovf eca| avg   | min   | max   | valid | total | ovf |
res_header_console+=$(printf "|%10s " "period")     # msg period, us (right-aligned, field-width=11)
res_header_console+=$(printf "|%10s " "msg rate")   # msg rate, KHz
res_header_console+=$(printf "|%10s " "data rate")  # data rate, Mbps
res_header_console+=$(printf "|%7s"  "vld eca")     # valid eca
res_header_console+=$(printf "|%7s"  "ovf eca")     # overflow eca
res_header_console+=$(printf "|%6s "  "avg")        # average delay, us
res_header_console+=$(printf "|%6s "  "min")        # min delay, us
res_header_console+=$(printf "|%6s "  "max")        # max delay, us
res_header_console+=$(printf "|%6s "  "valid")      # number of valid messages
res_header_console+=$(printf "|%6s "  "total")      # total messages
res_header_console+=$(printf "| %5s"  "ovf |")      # overflow flag

# timing message rates that should be measured
primary_msg_rates=(300 600 1000 1500 3000 6000) # fixed tmg msg rates [Hz]
adv_msg_rates=(10000 100000)             # advanced range of tmg msg rates [Hz]
all_msg_rates=() # all msg rates

tmg_msg_len=880  # timing message length [bits]

report_code() {
    # $1 - return code ($?)
    ret_code=$1
    if [ $ret_code -eq 0 ]; then
        echo "OK"
    else
        echo "FAIL ($ret_code)"
    fi
}

exit_on_fail() {
    # $1 - return code ($?)
    ret_code=$1
    if [ $ret_code -ne 0 ]; then
        echo "Exit!"
        exit 2
    fi
}

check_tr() {
    filenames="$fw_rxscu $script_rxscu"

    check_deployment $rxscu $filenames
}

setup_tr() {

    # $@ - sender IDs

    output=$(run_remote $rxscu \
        "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_TX $@")
    ret_code=$?

    echo -n " Setup TR: "
    report_code $ret_code
    exit_on_fail $ret_code

    # register senders (local injection by saft-ctl)
    register_senders "$@"
}

reset_tr_ecpu() {

    # $@ - sender IDs

    output=$(run_remote $rxscu \
        "source setup_local.sh && reset_node rx_node_dev SENDER_TX $@")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code

    # register senders (local injection by saft-ctl)
    register_senders "$@"
}

enable_tr_mps() {
    output=$(run_remote $rxscu \
        "source setup_local.sh && start_test4 \$rx_node_dev")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

disable_tr_mps() {
    output=$(run_remote $rxscu \
        "source setup_local.sh && stop_test4 \$rx_node_dev")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

setup_dm() {
    output=$(timeout 10 ssh "$login_dm" \
        "if [ ! -d "./$dst_test_dir" ]; then mkdir -p ./$dst_test_dir; fi")
    ret_code=$?
    if [ $ret_code -eq 0 ]; then
        output=$(scp $scp_opts "$src_test_dir/tools" "$src_test_dir/dm" \
            $login_dm:./$dst_test_dir/)
    else
        echo "FAIL ($ret_code): could not deploy '$dst_test_dir' on $datamaster. Exit!"
        exit 2
    fi
    echo -e "Test artifacts are deployed in '$datamaster:./$dst_test_dir'"
}

check_dm_schedule() {
    # complete an array with all timing message rates
    index=0
    for rate in ${primary_msg_rates[*]} ; do
        all_msg_rates[index]=$rate
        index=$(( $index + 1 ))
    done

    if [ "$is_msg_rate_limited" != "y" ]; then
        for first in ${adv_msg_rates[*]}; do
            step=$first               # iteration step
            last=$(( 10 * $first ))   # last value
            for rate in $(seq $first $step $last); do
                all_msg_rates[index]=$rate
                index=$(( $index + 1 ))
            done
        done
    fi

    # determine the depth of timing message block [messages with the same timestamp]
    # the depth of block is obtained from a given schedule filename
    # ie., depth of block is 1 for 'my_mps_rx_rate_1.dot'
    d_block=${sched_filename%%\.dot} # remove file suffix '.dot'
    d_block=${d_block##*_}           # remove all leading characters until last '_'
    d_block=$(( 10#$d_block ))       # convert string to decimal

    if [ $? -ne 0 ]; then
        echo "Error: Filename must end with numbers indicating msg blocks: $sched_filename. Exit"
        exit 2
    fi

    echo -e "Timing message block depth: $d_block [messages]"

    echo "Measurements will be done with following message rates [Hz]:"
    for rate in ${all_msg_rates[*]}; do
        echo $rate
    done
}

start_dm_schedule() {
    output=$(ssh "$login_dm" \
        "source ./${dst_test_dir}/tools/dm.sh && \
        set_value $sched_filename tperiod $t_period && \
        run_finite_schedule $sched_filename")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

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
    result_eca_delay \$rx_node_dev")
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

sender_ids() {
    # parse the 'parameter' attribute in DOT schedule file
    # and return the sender IDs

    # $1 - return/reply variable
    # $2 - schedule file path

    local -n ret="$1"
    local filepath="$2"
    local par_values=()

    if [ ! -f "$filepath" ]; then
        echo "Error: File not found: $filepath" >&2
        exit 1
    fi

    echo "Sender IDs in $filepath"
    while IFS= read -r line; do
        # extract the value of "par="
        val=$(printf "%s\n" "$line" | sed -n 's/.*par="\([^"]*\)".*/\1/p')

        if [[ -n "$val" ]]; then
            # extract the sender ID
            val=${val:2:12}

            # check if sender ID is known
            for v in ${par_values[@]}; do
                if [[ "$v" == "$val" ]]; then
                    val=""  # ID is kwown -> not needed
                    break
                fi
            done

            # add the sender ID to array
            if [[ -n "$val" ]]; then
                par_values+=("$val")
            fi
        fi
    done < "$filepath"

    printf "%s\n" "${par_values[@]}"

    ret=("${par_values[@]}")
}

register_senders() {

    # $@ - sender IDs

    # Register senders: inject timing messages with registration request locally (use saft-ctl)

    local channels=1
    if [[ "$fw_rxscu"=="fbas128.scucontrol.bin" ]]; then
        channels=8
    fi
    output=$(run_remote $rxscu \
        "source setup_local.sh && register_senders rx_node_dev $channels $@")
    ret_code=$?

    echo -n " Sender registration: "
    report_code $ret_code
    exit_on_fail $ret_code
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
    echo "  -m                     limited only with primary message rates"
    echo "  -h                     display this help and exit"
    echo
    echo "Example: $0 -s my_mps_rx_rate_1.dot -f $fw_rxscu"
}

unset username userpasswd sched_filename is_msg_rate_limited
unset OPTIND

while getopts 'hu:p:s:f:m' c; do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        s) sched_filename=$OPTARG; sched_filepath="$sched_dir/$sched_filename" ;;
        f) fw_rxscu=$OPTARG ;;
        m) is_msg_rate_limited="y" ;;
        *) usage; exit 1 ;;
    esac
done

# check if the specified schedule file exists
if [ ! -f "$sched_filepath" ]; then
    echo "Error: File not found: '$sched_filepath'. Exit"
    exit 1
fi

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$rxscu_name: " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username@$rxscu_name': " userpasswd
fi

# get name of an external file with DM schedule
if [ -z "$sched_filename" ]; then
    read -rp "DOT file with DM schedule: " sched_filename
fi

# setup everything
echo -e "\n--- 1. Set up DM=$datamaster ---\n"
setup_dm

echo -e "\n--- 2. Check deployment in TR=$rxscu_name ---\n"
check_tr

echo -e "\n--- 3. Check DM schedule in '$sched_filename' ---\n"
check_dm_schedule

echo -e "\n--- 4. Set up TR=$rxscu_name ---\n"
sender_ids ids "$sched_filepath" # extract the sender IDs from a given file
if [[ -n "${ids[@]}" ]]; then
    setup_tr "${ids[@]}"
fi

# start measurements
echo -e "\n--- 5. Start the measurements ---\n"

unset results
for i in ${!all_msg_rates[@]}; do

    rate=${all_msg_rates[$i]}
    t_msg=$(( 1000000000 / $rate ))               # period of single tmg msg [ns], not used
    t_period=$(( $d_block * 1000000000 / $rate )) # period of tmg msgs block [ns]

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
    start_dm_schedule

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
