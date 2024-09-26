#! /bin/bash -x

export TARGET=/common/usr/timing/stagging-fel0069/
export SOURCE=/common/usr/embedded/yocto/fesa/current/sdk/sysroots/core2-64-ffos-linux
ls -l $SOURCE/usr/lib/libboost_{graph,regex,serialization}.so.*
cp $SOURCE/usr/lib/libboost_{graph,regex,serialization}.so.* $TARGET/lib
cp $WORKSPACE/modules/ftm/lib/libcarpedm.so.* $TARGET/lib
cp $WORKSPACE/modules/ftm/bin/{dm-cmd,dm-sched} $TARGET/bin
