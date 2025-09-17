#!/bin/bash

# simulation tool
sim_tool=ghdl

# detect Linux distro
distro_local="unknown"
distro_deb=("debian" "ubuntu" "linuxmint")

print_distro_version() {
    distro_local=$(cat /etc/*-release | grep -oP "(?<=DISTRIB_ID=).*")
    echo $distro_local
}

check_ghdl_installation() {
    for d in "${distro_deb[@]}"; do
        if [ "${distro_local,,}" = "$d" ] ; then
            out=$(dpkg --list | grep $sim_tool)
            if [ "$out" != "" ]; then
                echo "$out"
            else
                echo "No $sim_tool found!"
                echo "Install it: sudo apt install $sim_tool"
            fi
            return
        fi
    done

    echo "no Debian-based distro! Exit"
    exit 1
}

print_distro_version
check_ghdl_installation