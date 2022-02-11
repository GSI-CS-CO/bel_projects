# !/bin/bash
###############################################################################
##                                                                           ##
##       Shell script which scans the /common/fesadata/data/scuxlXXXX        ##
## directory for core-dump-files and if found any invokes gdb and delete it. ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      pollcoredump.sh                                                ##
## Author:    Ulrich Becker                                                  ##
## Date:      11.02.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
CORE_EXTENTION="core"
SCAN_DIR=/common/fesadata/data/
POLL_INTERVAL_TIME=1

DEBUGGER=gdb

PROG_NAME=${0##*/}
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"


#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Shell script which scans the "/common/fesadata/data/scuxlXXXX" directory
for core-dump-files and if found any invokes gdb which creates a log-file
and deletes the huge core-dump-file.

The log-file has the same name like the executable with the extension ".log"
and is in the directory of the concerning SCU.

Usage:  $PROG_NAME [option] /path/to/the/executable
Author: Ulrich Becker

Options:

-h, --help
   This help and exit.

-i<seconds>
   Poll interval time in seconds. Default is $POLL_INTERVAL_TIME seconds.
   The value of 0 has a special meaning, in this case the scan is for
   one time only and the program will terminated.

-e --exit
   Terminates a possible already running instance of this application and exit self.

-s --show
   Shows the the content of the new generated log-file.

__EOH__
   exit 0
}

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

SHOW_LOGFILE=false
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
            A=${A:1}
         fi
         #shift
      ;;
      "e")
         DO_EXIT=true
      ;;
      "s")
         SHOW_LOGFILE=true
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
            "show")
               SHOW_LOGFILE=true
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


if [ ! "${HOSTNAME:0:5}" = "asl74" ]
then
   die "This script can run on ASL-cluster only!"
fi

if [ ! -d "$SCAN_DIR" ]
then
   die "Scan-directory \"$SCAN_DIR\" not found!"
fi

if [ ! -x "$(which "$DEBUGGER")" ]
then
   die "Debugger \"$DEBUGGER\" not found!"
fi

if [ ! -x "$1" ]
then
   die "Executable file with symbol-table \"$1\" not found!"
fi

#PID_NAME=$(basename $PROG_NAME)
#PID_NAME=/var/run/${PID_NAME%.*}.pid
PID_NAME=${PROG_NAME%.*}.pid
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

CORE_DUMP_FILE=$(basename $1 ).$CORE_EXTENTION
echo $CORE_DUMP_FILE

echo $$ > ${PID_NAME}
echo "PID-file \"${PID_NAME}\" created."

if [ "$POLL_INTERVAL_TIME" = "0" ]
then
   echo "Scanning directories \"${SCAN_DIR}scuxlXXXX\" for one time"
else
   echo "Scanning directories \"${SCAN_DIR}scuxlXXXX\" of a interval of $POLL_INTERVAL_TIME seconds"
fi
echo "for core-dump-files \"$CORE_DUMP_FILE\""

while true
do
   for i in $(ls ${SCAN_DIR}scuxl[0-9][0-9][0-9][0-9]/$CORE_DUMP_FILE 2>/dev/null )
   do
      LOGFILE="${i%.*}.log"
      if [ -f "$LOGFILE" ]
      then
         echo "Remove old log-file: \"$LOGFILE\""
         rm $LOGFILE
      fi
      echo "q" | $DEBUGGER $1 $i 2>/dev/null | tail -n8 | head -n6 > $LOGFILE
      if [ "$?" != "0" ]
      then
         die "Debugger failed!"
      fi
      if [ -f "$LOGFILE" ]
      then
         echo "Log-file \"$LOGFILE\" written"
         rm $i
         echo "Core-dump-file \"$i\" removed."
         if $SHOW_LOGFILE
         then
            echo "----------------------------------"
            cat $LOGFILE
            echo "----------------------------------"
         fi
      fi
   done
   if [ "$POLL_INTERVAL_TIME" = "0" ]
   then
      break
   fi
   sleep $POLL_INTERVAL_TIME
done

rm $PID_NAME
echo "done"

#=================================== EOF ======================================
