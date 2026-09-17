#! /bin/bash

# script to copy prerequisites and datamaster tools running on Yocto.
# copying to a stagging area at /common/usr/timing/
# the libraries are input for /common/export/timing-rte/generator-fel0069-yocto/loader.sh
# A: three boost libraries needed for libcarpedm
# B: dm-cmd, dm-sched, libcarpedm

# A works on ASL cluster, but not inside Jenkins job (Jenkins job has no access to /common/usr/
# B works on ASL cluster when the files are downloaded from Jenkins workspace of job 'YoctoBuild_Datamaster_Tools'
# B does not work inside Jenkins Job

# the staging area, the productive area is /common/export/timing-rte/generator-fel0069-yocto/
export TARGET=/common/usr/timing/stagging-fel0069/
# the source for the boost libraries
export SOURCE=/common/usr/embedded/yocto/fesa/current/sdk/sysroots/core2-64-ffos-linux
# the source for the datamaster tools
export DOWNLOAD=~/Downloads

# only for information:
ls -l $SOURCE/usr/lib/libboost_{graph,regex,serialization}.so.*
# A:
cp $SOURCE/usr/lib/libboost_{graph,regex,serialization}.so.* $TARGET/lib
# B:
cp $DOWNLOAD/libcarpedm.so.* $TARGET/lib
cp $DOWNLOAD/{dm-cmd,dm-sched} $TARGET/bin
# only for information
ls -l -R $TARGET
