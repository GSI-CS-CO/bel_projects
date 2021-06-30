#!/bin/bash

##########################
## Setup of the datamaster
##########################
# Pexaria as FBASTX, /dev/wbm0
# Pexp as FBASRX, /dev/wbm2

#####################################
## General setup for all timing nodes
#####################################

source setup.sh

export fbasdm="dev/wbm1"
export patt_loc="$HOME/gsi_prj/bel_projects/modules/fbas/x86"

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
    dm-cmd $fbasdm status -v
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

