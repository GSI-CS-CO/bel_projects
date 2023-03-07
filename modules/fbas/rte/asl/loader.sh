#!/bin/sh
# copy binaries

ARCH=$(/bin/uname -m)

cp $1/$ARCH/bin/* /usr/bin
cp $1/firmware/* /home/root
