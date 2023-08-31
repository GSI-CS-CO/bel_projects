#!/bin/bash

# get timing and fw info

unset username userpasswd scu option

usage() {
    echo "usage: $0 -u user -p password -s scu"
    echo
    echo "   scu - name of SCU (see below list of valid SCUs)"
    echo "         scuxl0264/305/321/329/339/411 in rack BG2A.A9"
    echo "         scuxl0396/497                 in rack BG2B.04"
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

if [ -z $scu ]; then
    usage
    exit 1
fi

domain=$(hostname -d)
eb_info_opts="-w"
eb_mon_timing_opts="-gyli"        # timing statistics, status
eb_mon_temp_opts="-t13 -f0x42"    # board temperature
wb_dev="dev/wbm0"

scu="$scu.$domain"
username="root"

# get username and password to access SCUs
if [ -z "$username" ]; then
    read -rp "username to access '${rxscu%%.*}, ${txscu%%.*}': " username
fi

if [ -z "$userpasswd" ]; then
    read -rsp "password for '$username' : " userpasswd
fi

echo -e "\n*** timing status of '${scu%%.*}' ***"
timeout 10 sshpass -p "$userpasswd" ssh $username@$scu "eb-mon $eb_mon_timing_opts $wb_dev"

echo -e "\n*** LM32 FW of '${scu%%.*}' ***"
timeout 10 sshpass -p "$userpasswd" ssh $username@$scu "eb-info $eb_info_opts $wb_dev"

echo -e "\n*** board temperature of '${scu%%.*}' ***"
timeout 10 sshpass -p "$userpasswd" ssh $username@$scu "eb-mon $eb_mon_temp_opts $wb_dev"

echo -e "\n*** artifacts (firmware, scripts) in '${scu%%.*}' ***"
timeout 10 sshpass -p "$userpasswd" ssh $username@$scu "ls -la fbas* *.sh; uptime"
