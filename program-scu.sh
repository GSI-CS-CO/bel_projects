#! /bin/bash

#quartus_jli="${QUARTUS:-/opt/quartus}/bin/quartus_jli"
quartus_jli="quartus_jli"
jam="$1"

if ! test -f "$jam"; then
  echo "Syntax: program-scu.sh <location-of-jam-file>"
  exit 1
fi

set -xe
"$quartus_jli" -aerase "$jam"
"$quartus_jli" -aerase -ddo_epcs_bulk_erase=1 "$jam"
"$quartus_jli" -ablankcheck "$jam"
"$quartus_jli" -aprogram "$jam"
"$quartus_jli" -averify "$jam"
