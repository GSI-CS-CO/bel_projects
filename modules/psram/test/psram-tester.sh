#!/bin/bash
set -eu

# Global variables
DEVICE="NULL"
TR="NULL"

# Run test
function run_test() {
  echo "Searching for PSRAM slave at $DEVICE ..."
  PSRAM_ADDR=$(eb-find "$DEVICE" 0x651 0x169edcb7)
  echo "Found PSRAM at $PSRAM_ADDR ..."

  if [ "$TR" = "scu4" ]; then
    echo "Selecting PSRAM #0 on $TR"
    saft-io-ctl tr0 -n PSRAM_SEL_0 -d 0
  fi

  echo "Creating test file ..."
  dd if=/dev/urandom of=put_file bs=33554432 count=1

  echo "Writing to PSRAM ..."
  eb-put "$DEVICE" "$PSRAM_ADDR" put_file

  echo "Reading back from PSRAM ..."
  eb-get "$DEVICE" "$PSRAM_ADDR/33554432" get_file

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
  echo "$0 -d dev/ttyUSB0 -t exploder5    - Test PSRAM on Exploder5"
  echo "$0 -d dev/wbm0 -t scu4            - Test PSRAM on SCU4"
  echo "$0 -h                             - Show help"
}

# Check given arguments
if [ $# -eq 0 ]; then
  echo "Error: Missing arguments!"
  print_help
  exit 1
fi

while getopts "d:t:h" opt; do
  case $opt in
    d)
      DEVICE="$OPTARG"
      ;;
    t)
      TR="$OPTARG"
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

if [ "$TR" = "NULL" ]; then
  echo "Error: Please provide a timing receiver type!"
  print_help
  exit 1
fi

# Check for dependencies
if ! command -v eb-find &>/dev/null || ! command -v eb-put &>/dev/null || ! command -v eb-get &>/dev/null || ! command -v saft-io-ctl &>/dev/null; then
  echo "Error: Required tools not found. Make sure 'eb-find', 'eb-put', 'eb-get', and 'saft-io-ctl' are installed and in the PATH."
  exit 1
fi

run_test

exit 0
