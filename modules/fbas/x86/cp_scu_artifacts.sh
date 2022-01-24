#!/bin/bash

# Copy LM32 firmware and scripts to listed SCUs

# MPS SCU - WR switch
# 396/497 - nwt0297
# 411/329 - nwt0471
# 321/264 - nwt0472
scu_arr=(scuxl0396 scuxl0497 scuxl0321 scuxl0264 scuxl0329 scuxl0411)

usage() {
    echo "usage: $0 scu"
    echo
    echo "   scu - name of SCU (eg., scuxl0264)"
    exit 1
}

if [ $# -eq 1 ]; then
    scu_arr=("$1")
fi

# project directory path
prj_dir="${PWD/fbas*/fbas}"

# actual gateware
#tr_gw="v6.0.1"
tr_gw="v6.1.2"

# SCU FW (independent from gateware version, built by devel host)
lm32_fw="fw/*.bin"

# SCU provided by user
domain=$(hostname -d)

# shell scripts to control MPS SCU
scu_tools="scu/*.sh"

# EB tool not available in ramdisk (eg., gw v6.0.1)
ebfwload="eb-fwload"

# user to access SCUs
username="root"

# Acknowledge SCUs
echo "SCUs below are chosen for MPS:"

for scu in ${scu_arr[*]}; do
    echo "$scu"
done

read -rp "Do you agree with above devices (Y/n)? " answer

if [ "$answer" != "Y" ] && [ "$answer" != "y" ] && [ "$answer" != "" ]; then
    echo "User disagreed. Exit!"
    exit 1
fi

# Prompt $username password for SCU
read -rsp "password for '$username' (SCU): " userpasswd

# Deploy artifacts

if [ "$tr_gw" == "v6.0.1" ]; then
    echo -e "\nDeploy '$lm32_fw', '$scu_tools' and '$ebfwload' to:"
else
    echo -e "\nDeploy '$lm32_fw' and '$scu_tools' to:"
fi

for item in ${scu_arr[*]}; do

    scu="${item}.$domain"

    # deploy EB tool (required for gw v6.0.1)
    if [ "$tr_gw" == "v6.0.1" ]; then
        SSHPASS="$userpasswd" timeout 10 sshpass -e scp "$tr_gw/$ebfwload" "$username@$scu:/usr/bin/"
    fi

    # deploy LM32 firmware
    SSHPASS="$userpasswd" timeout 10 sshpass -e scp "$prj_dir"/$lm32_fw "$username@$scu:~"

    # deploy script
    SSHPASS="$userpasswd" timeout 10 sshpass -e scp "$prj_dir"/$scu_tools "$username@$scu:~"

    echo "- $item"
done
