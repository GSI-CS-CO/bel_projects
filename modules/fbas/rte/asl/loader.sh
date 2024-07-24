#!/bin/sh
# copy binaries

ARCH=$(/bin/uname -m)

cp $1/$ARCH/bin/* /usr/bin  # test scripts
cp $1/firmware/* /home/root # firmware binaries
cp $1/test/*     /home/root # test artifacts
