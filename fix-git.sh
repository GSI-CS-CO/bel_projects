#! /bin/bash

do_submod () {
    git submodule sync
    git submodule init
    git submodule update --recursive
}

# top dir first
do_submod

# this is "false" because it does not work in ip_cores/wr-cores,
# due to wrong (old) submodule urls. And it looks like we are not using it.
if false; then
    # work in all sub-subdirs with submodules (recursive stops at 1st level)
    for f in $(find . -name .gitmodules | grep '/.*/.*/'); do
	(cd $(dirname $f) && do_submod)
    done

# use explicit names instead
else
    for d in ip_cores/wrpc-sw ip_cores/wr-cores ip_cores/fpga-config-space; do
	(cd $d && do_submod)
    done
fi
