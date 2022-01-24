#!/bin/bash

# get timing and fw info

usage() {
    echo "usage: $0 scu"
    echo
    echo "   scu - name of SCU (see below list of valid SCUs)"
    echo "         scuxl0264/305/321/329/339/411 in rack BG2A.A9"
    echo "         scuxl0396/497                 in rack BG2B.04"
    exit 1
}

if [ $# -ne 1 ]; then
    usage
fi

domain=$(hostname -d)
eb_info_opts="-w"
eb_mon_timing_opts="-gyli"        # timing statistics, status
eb_mon_temp_opts="-t13 -f0x42"    # board temperature
wb_dev="dev/wbm0"

scu="${1}.$domain"
username="root"

read -rsp "root password for SCU (${scu%%.*}): " password

echo -e "\n*** timing status of '${scu%%.*}' ***"
SSHPASS="$password" timeout 10 sshpass -e ssh "$username@$scu" "eb-mon $eb_mon_timing_opts $wb_dev"

echo -e "\n*** LM32 FW of '${scu%%.*}' ***"
SSHPASS="$password" timeout 10 sshpass -e ssh "$username@$scu" "eb-info $eb_info_opts $wb_dev"

echo -e "\n*** board temperature of '${scu%%.*}' ***"
SSHPASS="$password" timeout 10 sshpass -e ssh "$username@$scu" "eb-mon $eb_mon_temp_opts $wb_dev"

echo -e "\n*** artifacts (firmware, scripts) in '${scu%%.*}' ***"
SSHPASS="$password" timeout 10 sshpass -e ssh "$username@$scu" "ls -la *.bin *.sh; uptime"
