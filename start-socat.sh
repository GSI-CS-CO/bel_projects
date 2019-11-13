#/bin/sh
###############################################################################
##                                                                           ##
##  Shelscript to activate / deactivate the port forwarder "socat" in a SCU  ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    start-socat.sh                                                   ##
## Author:  Ulrich Becker                                                    ##
## Date:    13.11.2015                                                       ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"

PORT=60368
DEV=/dev/wbm0

die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

checkTarget()
{
   [ ! -n "$1" ] && die "Missing target URL!"
   ping -c1 $1 2>/dev/null 1>/dev/null
   [ "$?" != '0' ] && die "Target \"$1\" not found!"
}

if [ "$1" == "-h" ]
then
   echo "Tool for activating or deactivating the port forwarder \"socat\" to the" \
        "whisbone master \"${DEV}\" in a SCU"
   echo "Usage $0 [-k] <URL of target SCU>"
   echo "Option:"
   echo "-k  Deactivating of socat"
   exit 0
fi

if [ "$1" == "-k" ]
then
   checkTarget $2
   ssh root@${2} "killall socat"
   [ "$?" == "0" ] && echo "socat stopped"
   exit $?
fi

checkTarget $1

ssh root@${1} "/usr/bin/socat tcp-listen:$PORT,reuseaddr,fork file:$DEV </dev/null &"
[ "$?" == "0" ] && echo "socat started..."
exit $?

#=================================== EOF ======================================
