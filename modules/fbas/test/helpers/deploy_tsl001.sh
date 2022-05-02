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
build_dir="$module_dir/fw"
lm32_fw_dir="$module_dir/test/lm32"
scu_tools_dir="$module_dir/test/scu"
x86_tools_dir="$module_dir/test/tools"
x86_helpers_dir="$module_dir/test/helpers"
wrs_config_dir="$module_dir/test/wrs"
dm_schedule_dir="$module_dir/test/dm"

filename=$(basename "$0" .sh)
hostname=${filename##*_}

domain=$(hostname -d)
if [ "$domain" == "" ]; then
    echo -e "\nNo domain name was obtained. Probably you're outside of campus."
    echo -e "To skip a prompt for domain next time, invoke '$0 user <<< domain'\n"
    read -rp "Please provide domain for '$hostname': " domain
fi

hostname+=.$domain

# local deployment
rsync $rsync_opts --include='fbas.*.bin' --include='fbas16.*.bin' \
    --exclude='*' "$build_dir/" "$lm32_fw_dir/"

# remote deployment
rsync $rsync_opts \
    $lm32_fw_dir \
    $scu_tools_dir \
    $x86_tools_dir \
    $x86_helpers_dir \
    $wrs_config_dir \
    $dm_schedule_dir \
    --exclude="helpers/deploy_tsl001.sh" \
    "$1@$hostname:~/fbas/"
