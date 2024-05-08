#!/bin/bash

# Prerequisites: working ssh tunnel (eg., using sshuttle)

# Deploy test artifacts to the management host (ie., tsl101)
# - LM32 firmware:      fw/fbas*.scucontrol.bin
# - SCU tools/scripts:  scu/setup_local.sh

username="$USER"
hostname="tsl101"

usage() {
    echo "usage: $0 [OPTION]"
    echo
    echo "OPTION:"
    echo "  -u <username>          user name, if differs from $USER"
    echo "  -p <userpasswd>        user password"
    echo "  -s <hostname>          remote host name, by default $hostname"
    echo "  -h                     display this help and exit"
}

unset userpasswd

while getopts 'hyu:p:s:' c
do
    case $c in
        h) usage; exit 0 ;;
        u) username=$OPTARG ;;
        p) userpasswd=$OPTARG ;;
        s) hostname=$OPTARG ;;
        *) ;;
    esac
done

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
    echo -e "To skip a prompt for domain next time, invoke '$0 [OPTION] <<< domain'\n"
    read -rp "Please provide domain for '$hostname': " domain
fi

hostname+=.$domain

# local deployment
output=$(rsync $rsync_opts --include='fbas*.scucontrol.bin' \
    --include='sb_scan*.bin' --exclude='*' "$build_dir/" "$lm32_fw_dir/")

if [ $? -ne 0 ]; then
    echo "FAIL: cannot deploy '$build_dir' into '$lm32_fw_dir'"
fi

# remote deployment
echo "rsync $rsync_opts \
    ${module_dir}/test/ \
    --exclude=\"helpers/deploy_mngmnt_host.sh\" \
    $username@$hostname:$dest_path"

echo "Deploy the test setup to '$username@$hostname:$dest_path' ?"

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
