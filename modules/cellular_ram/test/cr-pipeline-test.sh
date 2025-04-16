#!/bin/bash
set -eu

# Global variables
DEVICE="NULL"
BYTE_SIZE=33554432
WAIT_SECONDS=0
FILE_WITH_ZEROS="no"
START_OFFSET="0x0"
READ_ONLY="no"

# Run test
function run_test() {
  echo "Searching for PSRAM slave at $DEVICE ..."
  PSRAM_ADDR=$(eb-find "$DEVICE" 0x651 0x169edcb8)
  echo "Found PSRAM at $PSRAM_ADDR ..."

  # Add offset, remove "0x"
  PSRAM_ADDR_CLEAN=${PSRAM_ADDR#0x}
  START_OFFSET_CLEAN=${START_OFFSET#0x}
  PSRAM_OFFSET_ADDR=$(echo "ibase=16; $PSRAM_ADDR_CLEAN + $START_OFFSET_CLEAN" | bc)
  PSRAM_ADDR=$(printf "0x%X" $PSRAM_OFFSET_ADDR)
  echo "Using PSRAM address $PSRAM_ADDR ..."

  if [ "$READ_ONLY" = "no" ]; then
    echo "Creating test file ..."
    if [ "$FILE_WITH_ZEROS" = "no" ]; then
      echo "Using random data ..."
      dd if=/dev/urandom of=put_file bs=$BYTE_SIZE count=1
    else
      echo "Using zeros ..."
      dd if=/dev/zero of=put_file bs=$BYTE_SIZE count=1
    fi

    echo "Writing to PSRAM ..."
    t_start_w=$(date +%s.%N)
    eb-put "$DEVICE" "$PSRAM_ADDR" put_file
    t_end_w=$(date +%s.%N)
    t_duration_w=$(echo "($t_end_w - $t_start_w) * 1000" | bc)
    printf "Writing took %.3f ms ...\n" $t_duration_w
    sleep $WAIT_SECONDS
  fi

  echo "Reading (back) from PSRAM ..."
  t_start_r=$(date +%s.%N)
  eb-get "$DEVICE" "$PSRAM_ADDR/$BYTE_SIZE" get_file
  t_end_r=$(date +%s.%N)
  t_duration_r=$(echo "($t_end_r - $t_start_r) * 1000" | bc)
  printf "Reading took %.3f ms ...\n" $t_duration_r

  if [ "$READ_ONLY" = "no" ]; then
    echo "Comparing files ..."
    if cmp -s put_file get_file; then
      echo "Test passed!"
    else
      echo "Test failed: Files are not identical."
    fi
  fi
}

# Print Help
function print_help() {
  echo "Example usage:"
  echo ""
  echo "$0 -d dev/ttyUSB0"
  echo "$0 -h"
}

# Check given arguments
if [ $# -eq 0 ]; then
  echo "Error: Missing arguments!"
  print_help
  exit 1
fi

while getopts "d:b:w:o:rzh" opt; do
  case $opt in
    d)
      DEVICE="$OPTARG"
      ;;
    b)
      BYTE_SIZE="$OPTARG"
      ;;
    w)
      WAIT_SECONDS="$OPTARG"
      ;;
    z)
      FILE_WITH_ZEROS="yes"
      ;;
    o)
      START_OFFSET="$OPTARG"
      ;;
    r)
      READ_ONLY="yes"
      ;;
    h)
      print_help
      exit 0
      ;;
    *)
      echo "Error: Invalid argument!"
      print_help
      exit 1
      ;;
  esac
done

# Check given arguments
if [ "$DEVICE" = "NULL" ]; then
  echo "Error: Please provide a device path!"
  print_help
  exit 1
fi

# Define an array of required tools
REQUIRED_TOOLS=("eb-find" "eb-put" "eb-get" "dd" "bc")

# Check for the presence of each tool in the array
for TOOL in "${REQUIRED_TOOLS[@]}"; do
  if ! command -v "$TOOL" &>/dev/null; then
    echo "Error: Required tool '$TOOL' not found. Make sure it is installed and in the PATH."
    exit 1
  fi
done

run_test

exit 0
