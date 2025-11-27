#!/bin/sh

##########################
## Setup of the datamaster
##########################

ttf_host="tsl014"

if [ "$HOSTNAME" = "$ttf_host" ]; then
    export patt_loc="fbas_test/dm"                         # pattern file location
    export fbasdm="dev/wbm0"                               # DM device
else
    export patt_loc="${PWD/fbas*/fbas/test/dm}"
    export fbasdm="dev/wbm1"
fi

export reg_maxmsg="0x41000a4"                              # DM register
export cmd_file_start_loop="start_synchron.dot"
export cmd_file_start_finite="start_synchron_finite.dot"
export sleep_period=25                                     # test duration, seconds

#####################
## Check DM device choice
#####################

check_fbasdm() {
    if [ -z "$fbasdm" ]; then
        echo "fbasdm is not set"
        exit 1
    fi
}

#####################
## Print remaining sleep time
#####################

wait_print_seconds() {
    # $1 - wait period in seconds

    if [ "$1" == "" ]; then
        return
    fi

    for i in $(seq 1 $1); do
        #echo -ne "time left (seconds): $[ $1 - $i ]\r"
        v=$(( $1 - $i ))
        v=$(printf "%*d\r" "8" $v)          # print numbers in 8 digits leading with spaces
        echo -ne "time left (seconds): $v"  # overwrite previous output
        sleep 1
    done
    echo

}

######################
## Show DM patterns
######################

print_dm_patt() {

    check_fbasdm

    dm-sched $fbasdm
}

######################
## Get DM sent message count (hex)
######################

cnt_dm_msg() {

    check_fbasdm

    status=$(dm-cmd $fbasdm rawstatus -v)   # get raw status

    # output looks like
    #CPU:00,THR:00,RUN:0
    #MSG:000196114              # msg count of thread0 in cpu0

    pattern="MSG:"              # match pattern
    msg_cnt=0

    for line in $status; do
        if [ -z "${line##*$pattern*}" ]; then   # if line contains a given pattern, 'MSG:'
            cnt=${line/MSG:/}                   # remove the pattern from a line
            cnt=$(echo $cnt | sed 's/^0*//')    # remove leading 0's
            if [ -z "$cnt" ]; then
                cnt=0
            fi
            #echo $cnt;
            msg_cnt=$(( $msg_cnt + $cnt ));     # sum it up
            #echo $msg_cnt;
        fi
    done
    printf "%x" $msg_cnt   # print the sum
}

######################
## Show DM status
######################

print_dm_diag() {

    check_fbasdm

    dm-cmd $fbasdm status -v
}

######################
## Clear DM status
######################

clear_dm_diag() {

    check_fbasdm

    dm-cmd $fbasdm cleardiag
}

######################
## Clear DM pattern
######################

clear_dm_patt() {

    check_fbasdm

    dm-sched $fbasdm clear
    #dm-sched $fbasdm
}

######################
## Load given pattern
######################

load_dm_patt() {

    check_fbasdm

    dm-sched $fbasdm add $patt_loc/$1
    #dm-sched $fbasdm
}

######################
## Start given pattern
######################

stop_dm_patt() {

    check_fbasdm

    dm-cmd $fbasdm stoppattern $1
    #dm-cmd $fbasdm status -v
}

######################
## Stop given pattern
######################

start_dm_patt() {

    check_fbasdm

    dm-cmd $fbasdm startpattern $1
}

######################
## Set max msg
######################

set_dm_maxmsg() {
    # $1 - maximum msgs in a frame (default 0x28)

    check_fbasdm

    eb-write $fbasdm $reg_maxmsg/4 $1
}

######################
## Start patterns synchronuous
######################

start_dm_synchron() {
    # $1 - file path with start command

    check_fbasdm

    dm-cmd $fbasdm -i $patt_loc/$1
}

######################
## Get a value of given variable
######################

set_value() {
    # $1 - external file with schedule (ie., my_mps_rx_rate_16.dot)
    # $2 - variable name (ie., pattern)
    # $3 - value
    # example: set_value my_mps_rx_rate_16.dot tperiod 33333

    act_tuple=$(grep -oE "${2}=([^,])+" $patt_loc/$1) # extract "pattern=value"
    new_tuple="$2=$3"                                 # set new value
    sed -i "s/$act_tuple/$new_tuple/" $patt_loc/$1    # edit in-place
}

######################
## Get a value of given variable
######################

get_value() {
    # $1 - external file with schedule (ie., my_mps_rx_rate_16.dot)
    # $2 - variable name (ie., pattern)

    line=$(grep -oE "${2}=([^,])+" $patt_loc/$1) # extract "pattern=value"
    value=${line#${2}=}
    echo "$value"

}

######################
## Run a schedule/pattern
######################

run_finite_schedule() {
    # $1 - external file with schedule (ie., my_mps_rx_rate_16.dot)

    if [ ! -f $patt_loc/$1 ]; then
        echo "'$patt_loc/$1' not found. Exit"
        return 1
    fi

    pattern=$(get_value $1 "pattern")

    if [ -z "$pattern" ]; then
        echo "Pattern not found. Exit"
        return 1
    fi

    tperiod=$(get_value $1 "tperiod")

    if [ -z "$tperiod" ]; then
        echo "tperiod not found. Exit"
        return 1
    fi

    qty=$(get_value $1 "qty")
    # trim a heading and trailing '"' character from value
    qty=${qty##\"}
    qty=${qty%%\"}

    if [ -z "$qty" ]; then
        echo "qty not found. Exit"
        return 1
    fi

    qty=$(( $qty + 1 ))                               # get final value
    sleep_period=$(( $tperiod * $qty / 1000000000 ))
    sleep_period=$(( $sleep_period + 1 ))             # prevent from a fraction of second
    echo "pattern=$pattern tperiod=$tperiod qty=$qty sleep_period=$sleep_period"

    clear_dm_diag
    clear_dm_patt
    load_dm_patt $1
    start_dm_patt $pattern
    echo "sleep $sleep_period" && wait_print_seconds $sleep_period
    cnt_dm_msg
}

start_loop_schedule() {
    # $1 - external file with schedule (ie., my_mps_basic_loop.dot)

    if [ ! -f $patt_loc/$1 ]; then
        echo "'$patt_loc/$1' not found. Exit"
        return 1
    fi

    pattern=$(get_value $1 "pattern")

    if [ -z "$pattern" ]; then
        echo "Pattern not found. Exit"
        return 1
    fi

    tperiod=$(get_value $1 "tperiod")

    if [ -z "$tperiod" ]; then
        echo "tperiod not found. Exit"
        return 1
    fi

    echo "pattern=$pattern tperiod=$tperiod"

    clear_dm_diag
    clear_dm_patt
    load_dm_patt $1
    start_dm_patt $pattern
}

stop_loop_schedule() {
    # $1 - external file with schedule (ie., my_mps_basic_loop.dot)

    if [ ! -f $patt_loc/$1 ]; then
        echo "'$patt_loc/$1' not found. Exit"
        return 1
    fi

    pattern=$(get_value $1 "pattern")

    if [ -z "$pattern" ]; then
        echo "Pattern not found. Exit"
        return 1
    fi

    echo "pattern=$pattern"

    stop_dm_patt $pattern
    #cnt_dm_msg
}

######################
## Run multiple patterns
######################

run_multi_patterns() {
    pattern="PatA"
    filename="my_mps_rx_rate_16"

    clear_dm_diag
    clear_dm_patt
    indices="a b c d"
    for index in $indices; do
        load_dm_patt "${filename}${index}.dot"
    done
    start_dm_synchron "$patt_loc/$cmd_file_start_finite"

    echo "sleep $sleep_period" && wait_print_seconds $sleep_period

    indices="A B C D"
    for index in $indices; do
        stop_dm_patt "Pat${index}"
    done

    cnt_dm_msg
}
