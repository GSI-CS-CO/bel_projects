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
lm32_fw="test/lm32"
scu_tools="test/scu"
x86_tools="test/tools"
x86_helpers="test/helpers"

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
    "$module_dir"/$scu_tools \
    "$module_dir"/$x86_tools \
    "$module_dir"/$x86_helpers \
    --exclude="helpers/deploy_tsl001.sh" \
    "$1@$hostname:~/fbas/"
