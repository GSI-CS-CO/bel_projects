#! /bin/bash
git submodule init
git submodule update --recursive
cd ip_cores/wrpc-sw
git submodule init
git submodule update --recursive
cd ../fpga-config-space
git submodule init
git submodule update --recursive
