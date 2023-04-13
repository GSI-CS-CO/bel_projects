#! /bin/sh

if [ "$#" -eq 0 ]; then
  echo "$0: required argument missing" >&2
  echo "Usage: $0 <folder with logs> [testmode]" >&2
  exit 1
fi
if [ 'testmode' = "$2" ] ; then
  echo "Running in test mode with stress/nice" >&2
fi
if ! [ -d "$1" ]; then
  echo "$1 is not a folder" >&2
  exit 1
fi

date
if [ 'testmode' = "$2" ] ; then
  ls $1/*gz 2> /dev/null > /dev/null
  if [ $? -eq 0 ] ; then
    echo 'uncompress, testmode'
    gunzip -r $1
  else
    echo 'compress, testmode'
    uptime
    #~ stress -t 30 -c 8 &
    stress -t 5 -c 8 &
    find $1 -type d -name .Generator*log* -prune -o -mtime +2 -type f -print -exec nice -n 18 gzip -9 {} +
    # nice -n 18 gzip -r -9 $1
    uptime
  fi
else
  echo 'compress older log files'
  find $1 -type d -name .Generator*log* -prune -o -mtime +2 -type f -print -exec nice -n 18 gzip -9 {} +
fi
date
