#! /usr/bin/bash
echo 'Installed Boost version'
grep -H 'BOOST_LIB_VERSION "' /usr/include/boost/version.hpp
if [ -z ${BOOSTPATH+x} ];
then echo "BOOSTPATH is unset";
else echo "BOOSTAPTH is set to '$BOOSTPATH'";
grep -H 'BOOST_LIB_VERSION "' $BOOSTPATH/include/version.hpp;
fi
if [ -f boost/version.hpp ];
then \
echo 'Local Boost version';
grep -H 'BOOST_LIB_VERSION "' boost/version.hpp;
fi
