#!/bin/bash

# Common variables and functions

# Directory setup
setup_dirs() {
    local abs_dir

    abs_dir=$(dirname "$(readlink -f "$0")") # absolute/full path to the called script
    source "$abs_dir/test_ttf_basic.sh" -s   # source the basic test script (contain other commons)

    # export common paths
    export abs_dir
    export sched_dir="${abs_dir%/*}/dm"      # directory with DM schedules
    export dst_test_dir="fbas_test"          # destination directory for DM scripts
    export src_test_dir="${abs_dir%/*}"      # source test directory
}

# Infrastructure setup
setup_infra() {

    export domain=$(hostname -d)             # domain name of local host
    export rxscu_name="scuxl0497"            # name of RX SCU
    export rxscu="$rxscu_name.$domain"       # full name of RX SCU, name=${rxscu%%.*}
    export datamaster="tsl014"               # Data Master
    export login_dm="root@$datamaster"       # pubkey login (alias 'backdoor') is used for login
    export mngmasters=( tsl101 )             # Management Masters

    export fw_rxscu="fbas128.scucontrol.bin" # default LM32 FW for RX SCU
    export tmg_msg_len=880                   # timing message length [bits]
}

# Setup schedule directory
setup_sched_dir() {

    local localhost=$(hostname -s)          # local host
    local mm is_localhost_mm="n"

    for mm in "${mngmasters[@]}"; do
        [[ "$localhost" == "$mm" ]] && {
            is_localhost_mm="y"
            break
        }
    done

    if [[ "$is_localhost_mm" == "n" ]]; then
        sched_dir="${PWD/fbas*/fbas}/test/dm"  # DM schedules directory in localhost
    fi
}

# SSH/SCP options detection
setup_ssh_opts() {
    export ssh_opts="-o StrictHostKeyChecking=no"
    export scp_opts="-r"                     # -r: recursive copy

    # Check if scp supports the '-O' option (use the legacy SCP protocol), required to access hosts, SCUs with older SSH
    if scp -O "$0" /dev/null &>/dev/null; then
        scp_opts+=" -O"
    fi
    export scp_opts
}

# Functions
report_code() {
    # $1 - return code ($?)

    if [ $1 -eq 0 ]; then
        echo "OK"
    else
        echo "FAIL ($1)"
    fi
}

exit_on_fail() {
    # $1 - return code ($?)

    if [ $1 -ne 0 ]; then
        echo "Exit on return code: $1!" >&2
        exit 2
    fi
}

check_tr() {
    local filenames

    filenames="$fw_rxscu $script_rxscu"
    check_deployment $rxscu $filenames
}

sender_ids() {
    # parse the 'parameter' attribute in DOT schedule file
    # and return the sender IDs

    # $1 - return/reply variable

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
        echo "FAIL ($ret_code): could not deploy '$dst_test_dir' on $datamaster. Exit!" >&2
        exit 2
    fi
    echo -e "Test artifacts are deployed in '$datamaster:./$dst_test_dir'"
}

check_dm_schedule() {
    # return an array with timing message rates

    # $1 - DM schedule filename
    # $2 - flag to enable high message rates
    # $3 - return variable for rate array
    # $4 - return variable for msg block

    local schedule_filename="$1"
    local enable_high_rates="$2"
    local -n msg_rates="$3"
    local -n msg_block="$4"

    # timing message rates used for measurements
    local std_rates=(100 300 600 1000 1500 3000 6000) # standard rates, Hz
    local high_rates=(10000 100000)                   # high rates, Hz
    local all_rates=()                                # all rates, Hz

    local index=0
    for rate in "${std_rates[@]}" ; do
        all_rates[index]=$rate
        index=$(( $index + 1 ))
    done

    if [ "$enable_high_rates" = "y" ]; then
        for first in "${high_rates[@]}"; do
            step=$first               # iteration step
            last=$(( 10 * $first ))   # last value
            for rate in $(seq $first $step $last); do
                all_rates[index]=$rate
                index=$(( $index + 1 ))
            done
        done
    fi

    # determine the timing message block depth: number of messages with the same timestamp
    # the block depth is obtained from a given schedule filename
    # ie., block depth is 1 for 'my_mps_rx_rate_1.dot'
    local block=${sched_filename%%\.dot} # remove file suffix '.dot'
    block=${block##*_}           # remove all leading characters until last '_'
    block=$(( 10#$block ))       # convert string to decimal

    if [ $? -ne 0 ]; then
        echo "Error: Filename must end with numbers indicating msg blocks: $sched_filename. Exit" >&2
        exit 2
    fi

    echo -e "Timing message block depth: $block [messages]"

    echo "Measurements will be done with following message rates [Hz]:"
    for rate in ${all_rates[*]}; do
        echo $rate
    done

    msg_rates=("${all_rates[@]}")
    msg_block="$block"
}

run_finite_dm_schedule() {
    output=$(ssh "$login_dm" \
        "source ./${dst_test_dir}/tools/dm.sh && \
        set_value $sched_filename tperiod $t_period && \
        run_finite_schedule $sched_filename")
    ret_code=$?
    report_code $ret_code
    exit_on_fail $ret_code
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

# Input validation & prompts

validate_sched_file() {
    # $1 - schedule file

    [[ -f "$1" ]] || {
        echo "Error: File not found: '$1'. Exit" >&2
        exit 1
    }

    [[ -s "$1" ]] || {
        echo "Error: File is empty: '$1'. Exit" >&2
        exit 2
    }
}

validate_user() {
    # $1 - name
    # $2 - password

    local -n username_ref="$1"
    local -n password_ref="$2"

    while [[ -z "$username_ref" ]]; do
        read -rp "username to access '$rxscu_name: " username_ref
    done

    # accept passwordless access
    if [[ -z "$password_ref" ]]; then
        read -rsp "password for '$1@$rxscu_name': " password_ref
        echo
    fi
}
