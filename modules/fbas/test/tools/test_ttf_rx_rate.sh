#!/bin/bash

# Test to determine the maximum data rate for receiver
# DM - pexaria28/32 (dev/wbm1, dev/wbm0) @tsl014
# RX SCU - scuxl0497

domain=$(hostname -d)             # domain name of local host
rxscu="scuxl0497.$domain"         # RX SCU
datamaster="tsl014"               # Data Master

fw_rxscu="fbas.scucontrol.bin"    # default LM32 FW for RX SCU

sched_dir="${PWD/fbas*/fbas}/dm"  # directory with DM schedules
dst_sched_dir="fbas/test/dm"      # destination directory for DM schedules

unset username userpasswd sched_filename

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Test to determine the maximum data rate for receiver"
    echo "Used SCUs: ${rxscu%%.*} (RX)"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -s <DM schedule>       external file with DM schedule"
    echo "  -f <LM32 firmware>     firmware binary file"
    echo "  -h                     display this help and exit"
}
while getopts 'hu:p:s:f:' c; do
    case $c in
        h) usage; exit 1 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        s) sched_filename=$OPTARG ;;
        f) fw_rxscu=$OPTARG ;;
    esac
done

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '${rxscu%%.*}: " username
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

echo -e "\nset up '${rxscu%%.*}'\n------------"
timeout 20 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx $fw_rxscu"

# enable MPS task of rxscu
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && read_counter \$DEV_RX && start_test4 \$DEV_RX"

# deploy the specified schedule file
echo -e "\ndeploy '$sched_filename'\n------------"
scp $sched_dir/$sched_filename $username@$datamaster:~/$dst_sched_dir/

# start a schedule on DM
echo -e "\nstart a schedule on '${datamaster}'---------"
ssh "$username@$datamaster" "source dm.sh && run_pattern $sched_filename"

echo "------------"

# disable MPX task of rxscu"
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && stop_test4 \$DEV_RX"
# print measurement results
timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && read_counters \$DEV_RX && result_ow_delay \$DEV_RX \$addr_cnt1"
