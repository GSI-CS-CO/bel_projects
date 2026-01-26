#!/bin/bash

# Used for the throughput measurement of the access layer. The measurement is based on the
# RFC2889 partial mesh 1:N test.

# XenaBay - perform RFC2889 partial mesh 1:16 test
# DM (pexaria28/32, dev/wbm{1/0} @tsl014) - generate DM broadcast with different rates

# Brief information about partial mesh 1:N is given below.
# It is used to determine the throughput, frame losses and forwarding rates of the switch
# when the stream is transmitted from 1-to-N or from N-to-1 port, measuring the capability
# of the switch device to forward frames without losses.

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

res_header_wiki="| *msg period, [us]* | *msg rate, [KHz]* | *data rate, [Mbps]* | *valid eca* | *overflow eca* | *avg messaging delay, [ns]* | *min messaging delay, [ns]* | *max messaging delay, [ns]* | *valid* | *total* | *overflow* |"
res_header_console="| t_period | msg rate | data rate | valid eca | ovf eca | avg | min | max | valid | total | ovf |"

# timing message rates that should be measured
dm_bc_rate=1000 # default timing msg rate [Hz]

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

sender_ids() {
    # parse the 'parameter' attribute in DOT schedule file
    # and return the sender IDs

    # $1 - return/reply variable

    local -n ret="$1"
    local par_values=()

    echo "Sender IDs in $sched_dir/$sched_filename"
    while IFS= read -r line; do
        # extract the value of "par="
        val=$(printf "%s\n" "$line" | sed -n 's/.*par="\([^"]*\)".*/\1/p')

        if [[ -n "$val" ]]; then
            # extract the sender ID
            val=${val:2:12}
            par_values+=("$val")
        fi
    done < "$sched_dir/$sched_filename"

    printf "%s\n" "${par_values[@]}"

    ret=("${par_values[@]}")
}

setup_tr() {
    # $@ - sender IDs

    output=$(run_remote $rxscu \
        "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_TX $@")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

reset_tr_ecpu() {
    output=$(run_remote $rxscu \
        "source setup_local.sh && reset_node rx_node_dev SENDER_TX $@")
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

start_dm_schedule() {
    output=$(ssh "$login_dm" \
        "source ./${dst_test_dir}/tools/dm.sh && \
        set_value $sched_filename tperiod $t_period && \
        start_loop_schedule $sched_filename")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

stop_dm_schedule() {

    output=$(ssh "$login_dm" \
        "source ./${dst_test_dir}/tools/dm.sh && \
        stop_loop_schedule $sched_filename")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
}

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

unset username userpasswd sched_filename
unset OPTIND

while getopts 'hu:p:r:s:f:' c; do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        r) dm_bc_rate=$OPTARG ;;
        s) sched_filename=$OPTARG ;;
        f) fw_rxscu=$OPTARG ;;
        *) usage; exit 1 ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '$rxscu_name: " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username@$rxscu_name': " userpasswd; echo
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

echo -e "\n--- 3. Set up TR=$rxscu_name ---\n"
sender_ids ids  # extract the sender IDs from $sched_dir/$sched_filename
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

# disable MPX task of rxscu"
echo -en " disable MPS operation of '$rxscu_name': "
disable_tr_mps

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
