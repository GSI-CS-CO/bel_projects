#!/bin/bash

# Terminate the current process from a sub-shell using 'SIGUSR1'.
# Type 'trap -l' to list all signals.
PROC=$$            # store the process ID
trap "exit -1" 10  # listen a SIGUSR1 by using trap

terminate_process() {
    echo "$@" >&2
    kill -10 $PROC
}

export PROC

# how to install the latest SAFT library and its tools (as of 1.7.2022):
# - change to bel_projects/ip_cores/saftlib
# - check out the latest version (git checkout v2.5.0)
# - compile and install (sudo make uninstall, make clean, make -j8, sudo make install)

# SAFT library specific variables
bg_proj_dir="$HOME/gsi_prj/bel_projects/modules/burst_generator" # project directory
bg_fw_file="$bg_proj_dir/burstgen.bin"                           # LM32 firmware binary
bg_saftbus_socket_path="/tmp/saftbus"                            # saftbus socket path
bg_tr="tr0"                                 # TR name for saftd
bg_out_io="IO1"                             # IO pin assigned for burst output
bg_msr_io="IO2"                             # IO pin used to measure the duration of burst
bg_evt_start_infinite="0xaaaa000000000000"
bg_evt_stop_infinite="0xbbbb000000000000"
bg_evt_start_run_once="0xfffe000000000001"
bg_evt_mask="0xffffffffffffffff"
bg_evt_flag=0                               # 0=new/overwrite burst, 1=append to burst

# device specific variables
bg_clock_master="dev/wbm2" # TR2 as PTP master
bg_generator="dev/wbm0"    # TR as burst generator
bg_buf_cmd="0x04060508"    # command buffer in shared memory of the bg_generator
bg_buf_state="0x0406050c"  # FSM state buffer

bg_cmd_config=1            # commands to set the FSM state
bg_cmd_startop=2
bg_cmd_stopop=3

# Burst parameters

# infinite burst, in nanoseconds
bg_b1_t_hi=10000000
bg_b1_t_p=15000000

# finite burst, in nanoseconds
bg_b2_t_hi=10000
bg_b2_t_p=20000
bg_b2_b_p=1440000           # burst period, (tested: 1240000 240000) 0=endless

# Other
cmd_output="/dev/stdout"    # default output

bg_usage() {
    echo "Tool to test the burst generation"
    echo
    echo "Bursts"
    echo "- infinite [ns]: width=$bg_b1_t_hi, period=$bg_b1_t_p"
    echo "- finite [ns]: width=$bg_b2_t_hi, period=$bg_b2_t_p, length=$bg_b2_b_p"
    echo
    echo "Setup"
    echo "- burst output: $bg_tr $bg_out_io"
    echo "- burst length: $bg_tr $bg_msr_io (acitve high during burst)"
    echo
    echo "Usage: $0 [option]"
    echo
    echo "[option]:"
    echo " -d <dev>  generate burst with a given TR, by default $bg_generator"
    echo " -c        select $bg_clock_master as clock master (assume, $bg_clock_master and $bg_generator are used here)"
    echo "           -c option is used only once after the host system startup"
    echo " -q        quiet mode"
    echo " -s        used to source $0"
    echo " -h        display this text and exit"
}

bg_select_clock_master() {
    # non-interactive way to run a given TR as a clock master

    result=$(ls /$bg_clock_master)
    if [ "$result" != "/$bg_clock_master" ]; then
        echo "$bg_clock_master is not found. Exit!"
        exit 1
    fi

    echo -n -e "\rptp master\r\r" | eb-console $bg_clock_master # set ptp to the master mode
    echo -n -e "\rptp start\r\r"  | eb-console $bg_clock_master # start ptp
    echo -n -e "ptp\r\r"  | eb-console $bg_clock_master         # show the current ptp status
    echo -e -n "time set $(date +%s) $(date +%N)\r\r" | eb-console $bg_clock_master  # update the WR time (synchronize with the system time)
    echo -e -n "time\r\r" | eb-console $bg_clock_master                              # show the WR time
}

bg_check_saftbus_socket_path () {
    # check saftlib socket path
    # $USER has no write permission for default /var/run/saftbus
    if [ -z "$SAFTBUS_SOCKET_PATH" ]; then
        export SAFTBUS_SOCKET_PATH="$bg_saftbus_socket_path"    # now $USER is able to load saftd
        echo "set saftbus socket path: $bg_saftbus_socket_path"
    fi
}

bg_check_saftd() {
    # check saftd socket path
    bg_check_saftbus_socket_path

    # load saftd if it's not running or it runs w/o a chosen TR
    tr_bg="$bg_tr:$bg_generator"  # chosen TR with burst generator

    # if saftd runs already, then check its command arguments
    pid=$(pidof saftd)
    if [ $? -eq 0 ]; then
        p_cmd=$(ps -p $pid -o args --no-headers)  # get process command
        p_cmd_args=${p_cmd##* }                   # get command arguments

        if [[ "$p_cmd_args" == *"$tr_bg"* ]]; then
            echo "saftd is available: $p_cmd"
            return
        fi

        # attach TR to saftd
        saft-ctl attach $tr_bg
        if [ $? -eq 0 ]; then
            echo "$tr_bg is attached to saftd."
            return
        else
            echo "$tr_bg could not be attached to $p_cmd. Exit!"
            exit 1
        fi
    fi

    # load saftd
    bg_load_saftd "$tr_bg"
}

bg_load_saftd() {
    # $1 - TR (tr0:dev/wbm0 or tr0:tcp/scuxl0304.acc.gsi.de)

    # check saftd socket path
    bg_check_saftbus_socket_path

    # load saftd
    pid=$(pidof saftd)
    if [ -z "$pid" ]; then
        echo "saftd is not available, loading 'saftd $@'"
        saftd "$@"
    fi

    # If HW watchdog still locks TR, then saftd cannot be re-loaded directly after its termination.
    # Hence, try to load saftd several times with a short interval.
    pause=3
    n_attempts=1

    pid=$(pidof saftd)
    while [ $n_attempts -lt 3 ] && [ -z "$pid" ]; do
        echo "wait for $pause seconds (attempt $n_attempts)"
        sleep $pause
        saftd "$@"
        pid=$(pidof saftd)
        n_attempts=$(( $n_attempts + 1 ))
    done

    # constructor of BurstGenerator checks the availability of FW by
    # resetting the LM32 eCPU, which takes also some time
    if [ -n "$pid" ]; then
        p_cmd=$(ps -p $pid -o args --no-headers)
        echo "saftd is loaded: $p_cmd ($pid)"
        pause=10
        echo "wait for $pause seconds until BurstGenerator gets ready"
        sleep $pause
    else
        echo "FAIL: saftd could not be loaded. Exit!"
        exit 1
    fi
}

bg_load_fw() {

    cd $bg_proj_dir                       # switch to the project directory
    eb-reset $bg_generator cpuhalt 0xff && \
    eb-fwload $bg_generator u 0 $bg_fw_file 1>$cmd_output  # halt target TR CPU and load a firmware
    eb-info -w $bg_generator 1>$cmd_output                 # show the firmware information
}

bg_show_bursts() {

    saft-burst-ctl tr0 -l 0
}

bg_delete_bursts() {

    saft-burst-ctl tr0 -x
}

bg_setup_infinite_burst() {
    # $1 - output pin name

    saft-burst-ctl tr0 -n $1 -b 2 -s $bg_evt_start_infinite -t $bg_evt_stop_infinite -v 1>$cmd_output
    saft-burst-ctl tr0 -b 2 -p $bg_b1_t_hi $bg_b1_t_p 0 0 $bg_evt_flag -v 1>$cmd_output
    saft-burst-ctl tr0 -e 2 1 -v 1>$cmd_output || terminate_process "Failed to set up the burst generator. Exit!"
}

bg_setup_run_once_burst() {
    # $1 - output pin name
    # exit if the launched tool returns failure

    saft-burst-ctl tr0 -n $1 -b 1 -s $bg_evt_start_run_once 1>$cmd_output
    saft-burst-ctl tr0 -b 1 -p $bg_b2_t_hi $bg_b2_t_p $bg_b2_b_p 0 $bg_evt_flag -v 1>$cmd_output
    saft-burst-ctl tr0 -e 1 1 1>$cmd_output || terminate_process "Failed to set up the burst generator. Exit!"
}

bg_set_bg_state() {
    # $1 - instruction code for the burst generator (LM32)

    saft-burst-ctl tr0 -i $1  1>$cmd_output # send an instruction code

    # wait until the desired state is set
    if [ $1 -ge $bg_cmd_config ] && [ $1 -le $bg_cmd_stopop ]; then
        expected_state=$(($1 + 2))
        if [ $1 -eq $bg_cmd_stopop ]; then
            expected_state=$1  # stop opready returns to state 3 'configured'
        fi
        state=$(saft-burst-ctl tr0 -S) # get the current state of the burst generator (LM32)
        timeout=5  # 5 seconds
        while [ $state -ne $expected_state ] && [ $timeout -gt 0 ]; do
            echo -e -n "wait: left $timeout seconds\r"
            sleep 1
            state=$(saft-burst-ctl tr0 -S)
            timeout=$(($timeout - 1))
        done

        if [ $state -ne $expected_state ]; then
            echo -e "\n bg state not changed: $state (expected $expected_state)"
            echo " burst cannot be started or stopped!"
            echo " (hint: clean the ECA queue)"
            return
        fi

        echo -e "\n bg state: '$state'" 1>$cmd_output
    else
        echo "invalid user command: $1 (valid: $bg_cmd_config, $bg_cmd_startop, $bg_cmd_stopop)"
    fi
}

bg_inject_event() {
    # $1 - event
    saft-ctl tr0 inject $1 0 0
}

bg_setup_msr_io() {
    # $1 - IO name (string)
    local io_name="$1"

    # form a pulse during infinite burst, set flag to 0xf to accept failed events (early, late, conflict, delayed)
    saft-io-ctl tr0 -n $io_name -u -c $bg_evt_start_infinite $bg_evt_mask 0 0xf 1 1>$cmd_output
    saft-io-ctl tr0 -n $io_name -u -c $bg_evt_stop_infinite $bg_evt_mask 0 0xf 0  1>$cmd_output

    # for a short pulse on finite burst
    saft-io-ctl tr0 -n $io_name -u -c $bg_evt_start_run_once $bg_evt_mask 0 0xf 1          1>$cmd_output
    saft-io-ctl tr0 -n $io_name -u -c $bg_evt_start_run_once $bg_evt_mask $bg_b2_b_p 0xf 0 1>$cmd_output
}

### test cases ###
bg_generate_infinite_burst() {
    bg_set_bg_state $bg_cmd_config

    bg_setup_infinite_burst $bg_out_io

    bg_set_bg_state $bg_cmd_startop

    ## set up extra IO
    bg_setup_msr_io "$bg_msr_io"

    echo "starting infinite burst at pin $bg_out_io"
    bg_inject_event $bg_evt_start_infinite

    # Words of the form $'string' are treated specially, backslash-escaped characters replaced as specified by the ANSI C standard.
    # Double-quoted string preceded by a dollar sign ($) will cause the string to be translated according to the current locale.
    read -rep "press 'Enter' to stop the burst > "

    echo "stop infinite burst at pin $bg_out_io"
    bg_inject_event $bg_evt_stop_infinite

    sleep 1
    bg_set_bg_state $bg_cmd_stopop
}

bg_generate_run_once_burst() {
    bg_set_bg_state $bg_cmd_config

    bg_setup_run_once_burst $bg_out_io

    bg_set_bg_state $bg_cmd_startop

    ## set up extra IO
    bg_setup_msr_io "$bg_msr_io"

    run_ival=$((bg_b2_b_p / 1000000000 + 1))
    echo "starting run_once burst at pin $bg_out_io, it will be stopped in $run_ival second(s)"

    bg_inject_event $bg_evt_start_run_once
    sleep $run_ival
    bg_set_bg_state $bg_cmd_stopop
}

bg_generate_all_bursts() {

    ## ask user to load firmware
    read -rp "load firmware [y/N]?: " answer

    if [ "$answer" == "y" ] || [ "$answer" == "Y" ]; then
        bg_load_fw

        # caveat: if FW is re-loaded, then saftd must be also re-loaded
        pid=$(pidof saftd)
        if [ $? -eq 0 ]; then

            # The watchdog locks TR for around 10 seconds to avoid connection of multiple saftd daemons.
            # Hence, after termination of saftd wait around 10 seconds letting watchdog to unlock TR.
            echo "Warning: terminating running saftd ..."
            killall saftd
            pause=10
            echo "wait for $pause seconds until watchdog unlocks TR"
            sleep $pause
        fi
    fi

    ## check saftd
    bg_check_saftd

    test_cases="bg_generate_infinite_burst bg_generate_run_once_burst"

    for tc in $test_cases; do
        ## prompt user to start next test
        echo -en "\nTest: $tc"
        read -rep " (press 'Enter' to start) > "

        ## run next test
        echo "starting '$tc'"
        eval "$tc"
        echo "completed '$tc'"
    done
}

# invoke 'source ./script.sh -s" to source a given script (do not run it!)
while getopts 'd:csvhq' opt; do
    case $opt in
        d) bg_generator=$OPTARG ;;               # generate bursts with a given TR
        c) bg_select_clock_master; exit 0 ;;     # select clock master
        s) source "sourced ${BASH_SOURCE[0]}" ;; # just source this script
        q) cmd_output="/dev/null" ;;             # set the quiet mode
        h) bg_usage; exit 0 ;;
        *) bg_usage; exit 1 ;;
    esac
done

bg_generate_all_bursts
