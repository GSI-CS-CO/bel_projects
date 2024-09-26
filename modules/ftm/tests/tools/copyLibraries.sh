#! /bin/bash

# script to copy prerequisites and datamaster tools running on Yocto.
# copying to a stagging area at /common/usr/timing/
# A: three boost libraries needed for libcarpedm
# B: dm-cmd, dm-sched, libcarpedm

# A works on ASL cluster, but not inside Jenkins job (Jenkins job has no access to /common/usr/
# B works on ASL cluster when the files are downloaded from Jenkins workspace of job 'YoctoBuild_Datamaster_Tools'
# B does not work inside Jenkins Job

export TARGET=/common/usr/timing/stagging-fel0069/
export SOURCE=/common/usr/embedded/yocto/fesa/current/sdk/sysroots/core2-64-ffos-linux
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
