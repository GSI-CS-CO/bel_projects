#!/bin/bash

# Project bistream archiving.
# Creates tarball <project>_<SHA>_<date>.tar.gz from project bistream files (sof, jic, rpd, bin)
# Call from inside .../<bel_projects_checkout>/syn/<platform>/<target>
#
# Aug 2018 by Mathias Kreider <m.kreider@gsi.de>


STR_PATH="Sure you are in a <bel_projects_checkout>/syn/<platform>/<target>/ folder ?"
TARGET=`cat ./Makefile | grep -Em1 "^\s*(TARGET)(\s|\=)" | cut -d\= -f2 | sed 's/\s*//g'`
TODAY=`date +%Y%m%d`
GITHASH=`git log -n1 --oneline | cut -d' ' -f1`
BINHASH=`grep --text -A2 "FW-ID ROM will contain:" $TARGET.bin | grep -Eo "^\s*[0-9a-f]+" | sed 's/\s*//g'`
EXTENSION=(".rpd" ".sof" ".jic" ".bin")
FILESARC=
TARARC="${TARGET}_${GITHASH}_${TODAY}.tar"


echo ""
if [[ $GITHASH != $BINHASH ]]; then
  echo "WARNING: current git log ($GITHASH) and firmware bin ($BINHASH) SHA's do not match"
  echo "Using current git SHA"
  echo ""
fi

if [ ! -f ./Makefile ]; then
  echo "ERROR: Found no Makefile"
  echo "$STR_PATH"
  exit 1
fi

if [ -z $TARGET ]; then
  echo "ERROR: Found no target platform in Makefile"
  echo "$STR_PATH"
  exit 1
else
  echo "Creating bistream archive for $TARGET"
fi

for EXT in ${EXTENSION[@]}; do
  INFILE="${TARGET}${EXT}"
  DATEDFILE="${TARGET}_${GITHASH}_${TODAY}${EXT}"
  if [ -f $INFILE ]; then
    cp $INFILE $DATEDFILE
    FILESARC+="$DATEDFILE "
  else
    echo "WARNING: $INFILE not found"
  fi
done

if [ ${#FILESARC[@]} -eq 0 ]; then
  echo "ERROR: Found no bitstreams to archive"
  exit 1
else
  tar -cvf $TARARC ${FILESARC[@]}
  gzip $TARARC
  rm ${FILESARC[@]}
fi