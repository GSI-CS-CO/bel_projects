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
rxscu="scuxl0497.$domain"         # RX SCU
datamaster="tsl014"               # Data Master
mngmaster="tsl001"                # Management Master
localhost=$(hostname -s)          # local host

fw_rxscu="fbas.scucontrol.bin"    # default LM32 FW for RX SCU

sched_dir="${PWD/fbas*/fbas}/dm"  # directory with DM schedules
if [ "$localhost" != "$mngmaster" ]; then
    sched_dir="${PWD/fbas*/fbas}/test/dm"
fi
dst_sched_dir="fbas/test/dm"      # destination directory for DM schedules

res_header_wiki="| *msg period, [us]* | *msg rate, [KHz]* | *data rate, [Mbps]* | *valid msg* | *overflow msg* | *average one-way delay, [ns]* | *min one-way delay, [ns]* | *max one-way delay, [ns]* | *valid msr* | *total msr* | *overflow* |"
res_header_console="| t_period | msg rate | data rate | valid msg | ovf msg | average | min | max | valid msr | total msr | ovf |"

# timing message rates that should be measured
primary_msg_rates=(300 600 1000 1200 1500 3000 6000) # fixed tmg msg rates [Hz]
adv_msg_rates=(10000 100000)             # advanced range of tmg msg rates [Hz]
all_msg_rates=() # all msg rates

tmg_msg_len=880  # timing message length [bits]

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
    echo "  -m                     limited only with primary message rates"
    echo "  -h                     display this help and exit"
    echo
    echo "Example (@tsl001): ./test_ttf_rx_rate.sh -s my_mps_rx_rate_1.dot -f fbas.scucontrol.bin"
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

echo "check deployment"
echo "----------------"

filenames="$fw_rxscu $script_rxscu"

for filename in $filenames; do
    timeout 10 sshpass -p "$userpasswd" ssh $username@$rxscu "if [ ! -f $filename ]; then echo $filename not found on ${rxscu}; exit 2; fi"
    result=$?
    report_check $result $filename $rxscu
done
echo

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

echo "Timing message block depth: $d_block [messages]"

echo "Measurements will be done with following message rates [Hz]:"
for rate in ${all_msg_rates[*]}; do
    echo $rate
done

echo -e "\nset up '${rxscu%%.*}'\n------------"
timeout 20 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && setup_mpsrx $fw_rxscu SENDER_ANY"

# deploy the specified schedule file
echo -e "\ndeploy '$sched_filename'\n------------"
scp $sched_dir/$sched_filename $username@$datamaster:~/$dst_sched_dir/

unset results
for rate in ${all_msg_rates[*]}; do

    t_msg=$(( 1000000000 / $rate ))               # period of single tmg msg [ns], not used
    t_period=$(( $d_block * 1000000000 / $rate )) # period of tmg msgs block [ns]

    echo "Measurement: msg rate=$rate tperiod=$t_period"
    echo "--------------------------------------------"

    # reset the FW in receiver node and enable MPS task
    echo -e "\nreset '${rxscu%%.*}'\n------------"
    timeout 20 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && reset_node DEV_RX SENDER_ANY"

    # enable MPS task of rxscu
    timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && start_test4 \$DEV_RX"

    # start a schedule on DM
    echo -e "\nstart a schedule on '${datamaster}'\n---------"
    ssh "$username@$datamaster" "source dm.sh && set_value $sched_filename tperiod $t_period && run_pattern $sched_filename"

    # disable MPX task of rxscu"
    timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && stop_test4 \$DEV_RX"
    echo

    # print measurement results
    counts=$(timeout 10 sshpass -p "$userpasswd" ssh "$username@$rxscu" "source setup_local.sh && read_counters \$DEV_RX && result_ow_delay \$DEV_RX")
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

    # break loop if the 'average one-way delay' is higher than 1 ms
    avg_owd=$(echo "$counts" | cut -d' ' -f4)
    avg_owd=$(( 10#$avg_owd ))     # convert a string to integer
    if [ $avg_owd -gt 1000000 ]; then
        break
    fi

done

echo "$sched_filename $fw_rxscu $localhost ($(date))"
echo "$res_header_console"
#echo "$res_header_wiki"
chars=${#res_header_console}
printf "%0.s-" $(seq 1 $chars) # one-liner to print a given number of '-' [1]
printf "\n"
echo -e "$results"
