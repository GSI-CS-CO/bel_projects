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

fw_rxscu="fbas16.scucontrol.bin"  # default LM32 FW for RX SCU

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

res_header_wiki="| *msg period, [us]* | *msg rate, [KHz]* | *data rate, [Mbps]* | *valid eca* | *overflow eca* | *avg messaging delay, [ns]* | *min messaging delay, [ns]* | *max messaging delay, [ns]* | *valid* | *total* | *overflow* |"
res_header_console="| t_period | msg rate | data rate | valid eca | ovf eca | average | min | max | valid | total | ovf |"

# timing message rates that should be measured
primary_msg_rates=(300 600 1000 1200 1500 3000 6000) # fixed tmg msg rates [Hz]
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
    output=$(run_remote $rxscu \
        "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_ANY")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

reset_tr_ecpu() {
    output=$(run_remote $rxscu \
        "source setup_local.sh && reset_node rx_node_dev SENDER_ANY")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
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

    # determine the depth of timing message block [messages]
    # the depth of block is obtained from a given schedule filename
    # ie., depth of block is 1 for 'my_mps_rx_rate_1.dot'
    d_block=${sched_filename%%\.dot} # remove file suffix '.dot'
    d_block=${d_block##*_}           # remove all leading characters until last '_'
    d_block=$(( 10#$d_block ))       # convert string to decimal

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
        run_pattern $sched_filename")
    ret_code=$?
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
        h) usage; exit 1 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        s) sched_filename=$OPTARG ;;
        f) fw_rxscu=$OPTARG ;;
        m) is_msg_rate_limited="y" ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$rxscu_name: " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

# get name of an external file with DM schedule
if [ -z "$sched_filename" ]; then
    read -rp "DOT file with DM schedule: " sched_filename
fi

# check if the specified schedule file exists
if [ ! -f $sched_dir/$sched_filename ]; then
    echo "'$sched_filename' not found in '$sched_dir'. Exit"
    exit 1
fi

# setup everything
echo -e "\n--- 1. Set up DM=$datamaster ---\n"
setup_dm

echo -e "\n--- 2. Check deployment in TR=$rxscu_name ---\n"
check_tr

echo -e "\n--- 3. Check DM schedule in '$sched_filename' ---\n"
check_dm_schedule

echo -e "\n--- 4. Set up TR=$rxscu_name ---\n"
setup_tr

# start measurements
echo -e "\n--- 5. Start the measurements ---\n"

unset results
for rate in ${all_msg_rates[*]}; do

    t_msg=$(( 1000000000 / $rate ))               # period of single tmg msg [ns], not used
    t_period=$(( $d_block * 1000000000 / $rate )) # period of tmg msgs block [ns]

    echo "Measurement: msg rate=$rate tperiod=$t_period"

    # reset the FW in receiver node
    echo -en " reset eCPU (LM32) of '$rxscu_name': "
    reset_tr_ecpu

    # enable MPS task of rxscu
    echo -en " enable MPS operation of '$rxscu_name': "
    enable_tr_mps

    # start a schedule on DM
    echo -en " start a schedule on '$datamaster': "
    start_dm_schedule

    # disable MPX task of rxscu"
    echo -en " disable MPS operation of '$rxscu_name': "
    disable_tr_mps

    # obtain stats from TR
    echo -en " obtain stats from '$rxscu_name': "
    counts=$(run_remote $rxscu \
        "source setup_local.sh && \
        read_counters \$rx_node_dev && \
        result_msg_delay \$rx_node_dev")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code

    counts=${counts//$'\n'/}           # remove all 'newline'
    counts=$(echo $counts | tr -s ' ') # remove consecutive spaces

    # format values
    t_period_float=$(printf "|%10.3f " "$((10**3 * $t_period/1000))e-3")           # message period [us]
    rate_float=$(printf "|%10.3f " "$((10**3 * $rate/1000))e-3")                   # message rate [KHz]
    d_rate_float=$(printf "|%10.3f" "$((10**3 * $rate*$tmg_msg_len/1000000))e-3")  # data rate [Mbps]
    sel_counts=$(echo $counts | cut -d' ' -f2-8)                                   # ignore 1st element (number of TX msgs)

    unset new_line
    new_line+=$t_period_float
    new_line+=$rate_float
    new_line+=$d_rate_float
    new_line=${new_line//./,}                # replace all 'dot' with 'comma' (decimal separator for floating-point numbers)
    new_line+=$(printf " | %s" $sel_counts)

    eca_valid=$(echo "$counts" | cut -d' ' -f2)
    eca_valid=$(( 10#$eca_valid ))  # convert a string to integer

    eca_overflow=$(echo "$counts" | cut -d' ' -f3)
    eca_overflow=$(( 10#$eca_overflow ))  # convert a string to integer
    if [ $eca_overflow -ne 0 ]; then
        new_line+=" | yes |\n"
    else
        new_line+=" | no |\n"
    fi

    results+=$new_line

    # break loop if the 'ECA overflow' counter has non-zero or
    # 'ECA valid' counter has zero value
    if [ $eca_overflow -ne 0 ] || [ $eca_valid -eq 0 ]; then
        break
    fi

    # break loop if the 'average messaging delay' is higher than 1 ms
    avg_owd=$(echo "$counts" | cut -d' ' -f4)
    avg_owd=$(( 10#$avg_owd ))     # convert a string to integer
    if [ $avg_owd -gt 1000000 ]; then
        break
    fi

done

echo -e "\n$datamaster:$sched_filename $rxscu:$fw_rxscu host:$localhost ($(date))\n"
echo "$res_header_console"
#echo "$res_header_wiki"
chars=${#res_header_console}
printf "%0.s-" $(seq 1 $chars) # one-liner to print a given number of '-' [1]
printf "\n"
echo -e "$results"
