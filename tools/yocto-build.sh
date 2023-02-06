#!/bin/bash

# Yocto stuff
unset LD_LIBRARY_PATH
source /common/usr/embedded/yocto/sdk/environment-setup-core2-64-ffos-linux

# Additional environment setup, in case you want to link against your own libetherbone you need to adjust this path
export LIBRARY_PATH=/common/usr/embedded/yocto/sdk/sysroots/core2-64-ffos-linux/usr/lib/

# Build
make clean
make USE_RPATH=no YOCTO_BUILD=yes C_INCLUDE_PATH=../ip_cores/etherbone-core/api:$(pwd)/../ip_cores/wrpc-sw/include:$(pwd)/../ip_cores/wrpc-sw/pp_printf
