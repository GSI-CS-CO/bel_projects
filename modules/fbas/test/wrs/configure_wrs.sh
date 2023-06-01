#!/bin/bash

# Configure the target WRS with the specified dot-config

usage() {
    echo "Usage: $0 username target config"
    echo
    echo "where:"
    echo "    username - username for login"
    echo "      target - target WRS name (eg, nwt0123m66)"
    echo "      config - path to dot-config file"
}

error() {
    echo "Missing arguments: $0 username target config. Exit!"
}

if [ $# -lt 3 ]; then
    usage
    exit 1
fi

domain="timing"
username=$1
target=${2}.$domain
config=$3

target_config_backup="${target}_dot-config.bak"
remote_config="/wr/etc/dot-config"

# ask an user password for remote ssh/scp access
read -rsp "please enter password for $username: " userpasswd

# check if configuration file of the target WRS is backed up
if [ ! -f "$target_config_backup" ]; then
    echo "back up the configuration of $target to $target_config_backup"
    SSHPASS="$userpasswd" sshpass -e scp $username@$target:$remote_config $target_config_backup
fi

# deploy the specified configuration file to the target WRS
echo "deploy $config to $target"
SSHPASS="$userpasswd" sshpass -e scp $config $username@$target:$remote_config

# prompt to reboot the target WRS (to apply new configuration)
read -rp "Reboot $target (Y/n)? " answer
if [ "$answer" == "" ]; then
    SSHPASS="$userpasswd" sshpass -e ssh $username@$target "/sbin/reboot"
fi
