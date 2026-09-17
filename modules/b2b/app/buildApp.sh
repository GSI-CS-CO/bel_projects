#!/bin/bash
# Example usage (asl74x)
# './buildApp.sh $HOME/consoleApps/b2b b2b-int-sys-mon b2b-client-sys'
# './buildApp.sh $HOME/consoleApps/b2b b2b-pro-sys-mon b2b-client-sys'
# './buildApp.sh $HOME/consoleApps/b2b b2b-int-transfer-mon b2b-mon'
# './buildApp.sh $HOME/consoleApps/b2b b2b-pro-transfer-mon b2b-mon'

SW=../x86
USRPATH=/common/usr/timing/user-dietrich/rocky9
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
cp $USRPATH/usr/lib/libdim.so $APPBUILD/lib
cp $USRPATH/usr/lib/libetherbone.so.5.1.3 $APPBUILD/lib
ln -sf $APPBUILD/lib/libetherbone.so.5.1.3 $APPBUILD/lib/libetherbone.so.5.1
ln -sf $APPBUILD/lib/libetherbone.so.5.1 $APPBUILD/lib/libetherbone.so.5
cd $APPBUILD
zip -r $NAME.zip bin lib
