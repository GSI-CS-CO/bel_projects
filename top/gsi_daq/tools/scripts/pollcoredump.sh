# !/bin/bash
###############################################################################
##                                                                           ##
##        Shell script which snoops for core-dump-files and saves it         ##
##                           on a NFS directory.                             ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      pollcoredump.sh                                                ##
## Author:    Ulrich Becker                                                  ##
## Date:      10.02.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
CORE_EXTENTION="core"
CORE_LOCATION_DIR=/
CORE_DESTINATION="/opt/fesa/data/"
POLL_INTERVAL_TIME=1

PROG_NAME=${0##*/}
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"

#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Shell script which snoops for coredump-files and saves it on a NFS directory.
The final target directory on ASL cluster of the core dump file will be "/common/fesadata/data/${HOSTNAME}/"
The core dump ability of the kernel will be enabled before as well.
NOTE: Unfortunately the command "ulimit" doesn't work global but only on the
      local process and its childs. 
      Therefore don't forget to invoke "ulimit -c unlimited" on the same process (or a base process)
      where the faulty application(s) will run.

Usage:  $PROG_NAME [option] &
Author: Ulrich Becker

Options:
-h, --help 
   This help and exit.

-i<seconds>
   Poll interval time in seconds. Default is $POLL_INTERVAL_TIME seconds.

-e
   Terminates a possible already running instance of this application and exit self.


Example for using gdb on ASL cluster:

   gdb /path/to/the/faulty/app/MyApp /common/fesadata/data/${HOSTNAME}/MyApp.$CORE_EXTENTION -tui

__EOH__
   exit 0
}

#------------------------------------------------------------------------------
printDocTagged()
{
   cat << __EOH__
<toolinfo>
        <name>$PROG_NAME</name>
        <topic>Development</topic>
        <description>
           Moves coredump-files in a NFS-directory
        </description>
        <usage>
           $PROG_NAME [-h --help] [-i{seconds}] [-e, --exit]
        </usage>
        <author>Ulrich Becker</author>
        <tags></tags>
        <version>1.0</version>
        <documentation></documentation>
        <environment>scu</environment>
        <requires>gdb</requires>
        <autodocversion>1.0</autodocversion>
</toolinfo>
__EOH__
   exit 0
}

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

DO_EXIT=false
while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
      "h")
          printHelp
      ;;
      "i")
         POLL_INTERVAL_TIME=${A:1}
         if [ ! -n "$POLL_INTERVAL_TIME" ]
         then
            shift
            POLL_INTERVAL_TIME=$1
         else
            while [ -n "$A" ]
            do
              A=${A:1}
            done
         fi
      ;;
      "e")
         DO_EXIT=true
      ;;
      "-")
         B=${A#*-}
         case ${B%=*} in
            "help")
               printHelp
            ;;
            "exit")
               DO_EXIT=true
            ;;
            "generate_doc_tagged")
               printDocTagged
            ;;
            *)
               die "Unknown long option \"-${A}\"!"
            ;;
         esac
         A=""
         ;;
      *)
        die "Unknown short option: \"${A:0:1}\"!"
      ;;
      esac
      A=${A#?}
   done
   shift
done

if [ "${HOSTNAME:0:5}" != "scuxl" ]
then
   die "This script can run on SCU only!"
fi

# Regular expressions seems not work on SCU! :-(
#if ! [[ "$POLL_INTERVAL_TIME" =~ ^[0-9]+$ ]]
#then
#   die "Wrong format of interval time: \"$POLL_INTERVAL_TIME\"!"
#fi

if [ ! -d "$CORE_LOCATION_DIR" ]
then
   die "Temporary ram-disc directory \"$CORE_LOCATION_DIR\" for core dump file not found!"  
fi

if [ ! -d "$CORE_DESTINATION" ]
then
   die "NFS destination directory \"$CORE_DESTINATION\" not found!"
fi

PID_NAME=$(basename $PROG_NAME)
PID_NAME=/var/run/${PID_NAME%.*}.pid
if [ -f "$PID_NAME" ]
then
   echo "Trying to kill already running application."
   kill $(cat $PID_NAME)
   rm $PID_NAME
fi
if $DO_EXIT
then
   exit 0
fi

# Seems to work only on this process and its child processes! Sch..... :-(
ulimit -c unlimited
if [ "$?" != "0" ]
then
   die "Can't enable the core dump ability!" 
fi

echo "${CORE_LOCATION_DIR}%e.${CORE_EXTENTION}" > /proc/sys/kernel/core_pattern

echo $$ > ${PID_NAME}
echo "PID-file \"${PID_NAME}\" created."
echo "Call \"kill \$(cat ${PID_NAME})\" for terminating."

echo "Final target of core-dump file on ASL cluster is: \"/common/fesadata/data/${HOSTNAME}/\"."
echo "Snooping for core dump files in directory: \"$CORE_LOCATION_DIR\", interval: $POLL_INTERVAL_TIME seconds. ..." 
while true 
do
   sleep $POLL_INTERVAL_TIME
   for i in $(ls ${CORE_LOCATION_DIR}*.${CORE_EXTENTION} 2>/dev/null )
   do
      echo "Moving core dump: \"$i\" to \"$CORE_DESTINATION\""
      cp $i $CORE_DESTINATION
      rm $i
   done
done

#=================================== EOF ======================================
