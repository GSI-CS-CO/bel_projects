#! /usr/bin/bash

# script to extract BOOST_LIB_VERSION from version.hpp.
# Tries thre places:
# /usr/include/boost/version.hpp  (the installed Boost instance)
# $BOOSTPATH/include/boost/version.hpp  (the instance at BOOSTPATH)
# boost/version.hpp  (if script runs in the root folder of a boost instance)

echo 'Installed Boost version'
grep -H 'BOOST_LIB_VERSION "' /usr/include/boost/version.hpp
if [ -z ${BOOSTPATH+x} ];
then echo "BOOSTPATH is unset";
else echo "BOOSTPATH is set to '$BOOSTPATH'";
grep -H 'BOOST_LIB_VERSION "' $BOOSTPATH/include/boost/version.hpp;
fi
if [ -f boost/version.hpp ];
then \
echo 'Local Boost version';
grep -H 'BOOST_LIB_VERSION "' boost/version.hpp;
else echo 'No local Boost version';
fi
