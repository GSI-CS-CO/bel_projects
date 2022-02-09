#!/bin/bash

##########################
## Setup of the datamaster
##########################
# Pexaria as DM, /dev/wbm1 on devel host
# Pexaria as FBASTX, /dev/wbm0
# Pexp as FBASRX, /dev/wbm2

#####################################
## General setup for all timing nodes
#####################################

source setup.sh

export fbasdm="dev/wbm1"                                   # DM device on devel host
export patt_loc="${PWD/fbas*/fbas}/test/dm"                # DM pattern location
export reg_maxmsg="0x41000a4/4"                            # DM register
export cmd_file_start_loop="start_synchron.dot"
export cmd_file_start_finite="start_synchron_finite.dot"

function check_fbasdm() {
    if [ -z "$fbasdm" ]; then
        echo "fbasdm is not set"
        exit 1
    fi
}

######################
## Show DM patterns
######################

function print_dm_patt() {

    check_fbasdm

    dm-sched $fbasdm
}

######################
## Get DM sent message count (hex)
######################

function cnt_dm_msg() {

    check_fbasdm

    status=$(dm-cmd $fbasdm rawstatus -v)   # get raw status

    # output looks like
    #CPU:00,THR:00,RUN:0
    #MSG:000196114              # msg count of thread0 in cpu0

    pattern="MSG:"              # match pattern
    msg_cnt=0

    for line in $status; do
        if [[ "$line" =~ "MSG" ]]; then       # line has a 'pattern'
            cnt=${line/MSG:/};                # remove 'pattern' from a line
            cnt=$((10#$cnt));                 # convert it to decimal
            #echo $cnt;
            msg_cnt=$(($msg_cnt + $cnt));     # sum it up
            #echo $msg_cnt;
        fi
    done
    echo "$(printf '%08x' $msg_cnt)"          # convert to hexadecimal
}

######################
## Show DM status
######################

function print_dm_diag() {

    check_fbasdm

    dm-cmd $fbasdm status -v
}

######################
## Clear DM status
######################

function clear_dm_diag() {

    check_fbasdm

    dm-cmd $fbasdm cleardiag
}

######################
## Clear DM pattern
######################

function clear_dm_patt() {

    check_fbasdm

    dm-sched $fbasdm clear
    #dm-sched $fbasdm
}

######################
## Load given pattern
######################

function load_dm_patt() {

    check_fbasdm

    dm-sched $fbasdm add $patt_loc/$1
    #dm-sched $fbasdm
}

######################
## Start given pattern
######################

function stop_dm_patt() {

    check_fbasdm

    dm-cmd $fbasdm stoppattern $1
    #dm-cmd $fbasdm status -v
}

######################
## Stop given pattern
######################

function start_dm_patt() {

    check_fbasdm

    dm-cmd $fbasdm startpattern $1
}

######################
## Set max msg
######################

function set_dm_maxmsg() {
    # $1 - maximum msgs in a frame (default 0x28)

    check_fbasdm

    eb-write $fbasdm $reg_maxmsg $1
}

######################
## Start patterns synchronuous
######################

function start_dm_synchron() {
    # $1 - file path with start command

    check_fbasdm

    dm-cmd $fbasdm -i $1
}
