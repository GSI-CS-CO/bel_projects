#!/bin/bash

# Prerequisites: working ssh tunnel (eg., using sshuttle)

# Deploy test artifacts to the management host (ie., tsl101)
# - LM32 firmware:      fw/fbas.scucontrol.bin
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

username=$1
hostname=tsl001
dest_path="./fbas_test"

rsync_opts="--numeric-ids -Pauvh"
scp_opts="-r -O"

module_dir="${PWD/fbas*/fbas}"     # bel_projects/modules/fbas
build_dir="$module_dir/fw"
lm32_fw_dir="$module_dir/test/lm32"
scu_tools_dir="$module_dir/test/scu"
x86_tools_dir="$module_dir/test/tools"
x86_helpers_dir="$module_dir/test/helpers"
wrs_config_dir="$module_dir/test/wrs"
dm_schedule_dir="$module_dir/test/dm"

domain=$(hostname -d)
if [ "$domain" == "" ]; then
    echo -e "\nNo domain name was obtained. Probably you're outside of campus."
    echo -e "To skip a prompt for domain next time, invoke '$0 user <<< domain'\n"
    read -rp "Please provide domain for '$hostname': " domain
fi

hostname+=.$domain

# local deployment
echo "local synchronization"
rsync $rsync_opts --include='fbas.*.bin' --include='fbas16.*.bin' \
    --include='sb_scan*.bin' --exclude='*' "$build_dir/" "$lm32_fw_dir/"

# remote deployment
echo "set up the management host $hostname"

echo "rsync $rsync_opts \
    ${module_dir}/test/ \
    --exclude=\"helpers/deploy_mngmnt_host.sh\" \
    $username@$hostname:$dest_path"

read -p "press any key to continue, or CTRL+c to exit"

rsync $rsync_opts \
    ${module_dir}/test/ \
    --exclude="helpers/deploy_mngmnt_host.sh" \
    $username@$hostname:$dest_path

if [ $? != 0 ]; then
    echo "rsync fails -> let's try with scp"
    echo "ssh $ssh_opts $username@$hostname \"mkdir -p $dest_path\""
    echo "scp $scp_opts $module_dir/test/* $username@$hostname:$dest_path"
    read -p "press any key to continue, or CTRL+c to exit"
    ssh $ssh_opts $username@$hostname "mkdir -p $dest_path" && \
    scp $scp_opts ${module_dir}/test/* $username@$hostname:$dest_path
fi
