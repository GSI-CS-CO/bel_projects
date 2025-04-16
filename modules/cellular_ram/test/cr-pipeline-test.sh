#!/bin/bash
set -eu

# Global variables
DEVICE="NULL"
BYTE_SIZE=33554432

# Run test
function run_test() {
  echo "Searching for PSRAM slave at $DEVICE ..."
  PSRAM_ADDR=$(eb-find "$DEVICE" 0x651 0x169edcb8)
  echo "Found PSRAM at $PSRAM_ADDR ..."
  echo "Creating test file ..."
  dd if=/dev/urandom of=put_file bs=$BYTE_SIZE count=1

  echo "Writing to PSRAM ..."
  eb-put "$DEVICE" "$PSRAM_ADDR" put_file

  echo "Reading back from PSRAM ..."
  eb-get "$DEVICE" "$PSRAM_ADDR/$BYTE_SIZE" get_file

  echo "Comparing files ..."
  if cmp -s put_file get_file; then
    echo "Test passed!"
  else
    echo "Test failed: Files are not identical."
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

while getopts "d:b:h" opt; do
  case $opt in
    d)
      DEVICE="$OPTARG"
      ;;
    b)
      BYTE_SIZE="$OPTARG"
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
REQUIRED_TOOLS=("eb-find" "eb-put" "eb-get")

# Check for the presence of each tool in the array
for TOOL in "${REQUIRED_TOOLS[@]}"; do
  if ! command -v "$TOOL" &>/dev/null; then
    echo "Error: Required tool '$TOOL' not found. Make sure it is installed and in the PATH."
    exit 1
  fi
done

run_test

exit 0
