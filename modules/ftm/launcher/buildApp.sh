#!/bin/bash
# Example usage (asl7xy)
# './buildApp.sh . dm-monitor-int dm-monitor'

ETHERBONE=../../../ip_cores/etherbone-core/api
ifeq ($(findstring asl75,$(shell hostname)),asl75)
  BOOST =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  BOOST =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
  BOOST = /lib/x86_64-linux-gnu
endif
endif
BOOST=

APPBUILD=$1/$2/current
NAME=$2
BIN=$3

echo 'build path is' $APPBUILD

echo 'cleaning ...'
rm -r $APPBUILD/$NAME.zip $APPBUILD/bin/ $APPBUILD/lib/

echo 'building ...'
cp ../bin $APPBUILD/bin
cp start-app-$NAME.sh $APPBUILD/bin
cp $ETHERBONE/.libs/libetherbone.so.5.1.2 $APPBUILD/lib
cp $BOOST/lib/libboost_graph.so $APPBUILD/lib
cp $BOOST/lib/libboost_serialization.so $APPBUILD/lib
cp $BOOST/lib/libboost_regex.so $APPBUILD/lib
# ln -sf $APPBUILD/lib/libetherbone.so.5.1.2 $APPBUILD/lib/libetherbone.so.5.1
# ln -sf $APPBUILD/lib/libetherbone.so.5.1 $APPBUILD/lib/libetherbone.so.5
# cd $APPBUILD
zip -r $NAME.zip bin lib
