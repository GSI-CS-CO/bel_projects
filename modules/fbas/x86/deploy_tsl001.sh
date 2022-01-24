#!/bin/bash

# Prerequisites: working ssh tunnel (eg., using sshuttle)

# Deploy artifacts into tsl001
# - LM32 firmware:      fw/fbas.bin
# - SCU tools/scripts:  scu/setup_local.sh

usage() {
    echo "Error: missing an user argument"
    echo "Usage: $0 user"
    echo "Exit!"
}

if [ $# -ne 1 ]; then
    usage
    exit 1
fi

module_dir="${PWD/fbas*/fbas}"     # bel_projects/modules/fbas
rsync_opts="-Pauvh"
lm32_fw="fw/*.bin"
scu_tools="scu/*.sh"
x86_tools="x86/*.sh"

filename=$(basename "$0" .sh)
hostname=${filename##*_}

domain=$(hostname -d)
if [ "$domain" == "" ]; then
    echo -e "\nNo domain name was obtained. Probably you're outside of campus."
    echo -e "To skip a prompt for domain next time, invoke '$0 user <<< domain'\n"
    read -rp "Please provide domain for '$hostname': " domain
fi

hostname+=.$domain

rsync $rsync_opts \
    "$module_dir"/$lm32_fw \
    "$1@$hostname:~/fbas/fw/"

rsync $rsync_opts \
    "$module_dir"/$scu_tools \
    "$1@$hostname:~/fbas/scu/"

rsync $rsync_opts \
    "$module_dir"/$x86_tools \
    "$1@$hostname:~/fbas/x86/"
