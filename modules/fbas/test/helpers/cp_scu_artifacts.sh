#!/bin/bash

# Copy LM32 firmware and scripts to listed SCUs

# MPS SCU - WR switch
# 396/497 - nwt0297
# 411/329 - nwt0471
# 321/264 - nwt0472
scu_arr=(scuxl0396 scuxl0497 scuxl0321 scuxl0264 scuxl0329 scuxl0411)

unset username userpasswd scu option answer

usage() {
    echo "usage: $0 [OPTION]"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name to log in to SCUs"
    echo "  -p <userpasswd>        user password"
    echo "  -s <scu>               SCU name"
    echo "  -y                     'yes' to all prompts"
    echo "  -h                     display this help and exit"
    exit 1
}

while getopts 'hyu:p:s:' c
do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        s) scu=$OPTARG ;;
        y) option="auto" ;;
    esac
done

if [ -n "$scu" ]; then
    scu_arr=("$scu")
fi

# project directory path
prj_dir="${PWD/fbas*/fbas}"

# actual gateware
#tr_gw="v6.0.1"
tr_gw="v6.1.2"

# SCU FW (independent from gateware version, built by devel host)
lm32_fw="lm32/fbas*.scucontrol.bin lm32/sb_scan.scucontrol.bin"

# SCU provided by user
domain=$(hostname -d)

# shell scripts to control MPS SCU
scu_tools="scu/*.sh scu/*.sched"

# EB tool not available in ramdisk (eg., gw v6.0.1)
ebfwload="eb-fwload"

# Acknowledge SCUs
echo "SCUs below are chosen for MPS:"

for scu in ${scu_arr[*]}; do
    echo "- $scu"
done

if [ "$option" != "auto" ]; then
    read -rp "Do you agree with above devices (Y/n)? " answer

    if [ "$answer" != "Y" ] && [ "$answer" != "y" ] && [ "$answer" != "" ]; then
        echo "User disagreed. Exit!"
        exit 1
    fi
fi

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '${rxscu%%.*}, ${txscu%%.*}': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

# Deploy artifacts
echo -e "\nArtifacts to be deployed:"
echo "-" $lm32_fw
echo "-" $scu_tools

if [ "$tr_gw" == "v6.0.1" ]; then
    echo "-" $ebfwload
fi

# Start deployment
echo -e "\nStart deployment ..."
for item in ${scu_arr[*]}; do

    scu="${item}.$domain"

    # deploy LM32 firmware and script (to show progress: redirect output and grep)
    SSHPASS="$userpasswd" sshpass -e scp -pv "$prj_dir"/$lm32_fw "$prj_dir"/$scu_tools "$username@$scu:~" 2>&1 | grep -v debug

    # deploy EB tool (required for gw v6.0.1)
    if [ "$tr_gw" == "v6.0.1" ]; then
        SSHPASS="$userpasswd" sshpass -e scp -pv "$prj_dir/lm32/$tr_gw/$ebfwload" "$username@$scu:/usr/bin/" 2>&1 | grep -v debug
    fi

done
