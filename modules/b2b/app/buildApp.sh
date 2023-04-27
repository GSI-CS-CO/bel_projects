#!/bin/bash
# Example usage (asl74x)
# './buildApp.sh $HOME/consoleApps/b2b b2b-int-sys-mon b2b-client-sys'
# './buildApp.sh $HOME/consoleApps/b2b b2b-pro-sys-mon b2b-client-sys'
# './buildApp.sh $HOME/consoleApps/b2b b2b-int-transfer-mon b2b-mon'
# './buildApp.sh $HOME/consoleApps/b2b b2b-pro-transfer-mon b2b-mon'




EB=../../../ip_cores/etherbone-core/api
SW=../x86
DIM=/common/usr/timing/b2b/yocto/dim_v20r33/
APPBUILD=$1/$2/current
NAME=$2
BIN=$3

echo 'build path is' $APPBUILD

echo 'cleaning ...'
rm $APPBUILD/$NAME.zip
rm $APPBUILD/bin/*
rm $APPBUILD/lib/*

echo 'building ...'

cp $SW/$BIN $APPBUILD/bin
cp start-app-$NAME.sh $APPBUILD/bin
cp $SW/libb2blib.so.1.0 $APPBUILD/lib
ln -sf $APPBUILD/lib/libb2blib.so.1.0 $APPBUILD/lib/libb2blib.so.1
ln -sf $APPBUILD/lib/libb2blib.so.1 $APPBUILD/lib/libb2blib.so
cp $DIM/linux/libdim.so $APPBUILD/lib
cp $EB/.libs/libetherbone.so.5.1.2 $APPBUILD/lib
ln -sf $APPBUILD/lib/libetherbone.so.5.1.2 $APPBUILD/lib/libetherbone.so.5.1
ln -sf $APPBUILD/lib/libetherbone.so.5.1 $APPBUILD/lib/libetherbone.so.5
cd $APPBUILD
zip -r $NAME.zip bin lib
