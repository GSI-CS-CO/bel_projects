#!/bin/bash
set -eu

# Global variables
DEVICE="NULL"
BYTE_SIZE=33554432
WAIT_SECONDS=0
FILE_WITH_ZEROS="no"
ARG_START_OFFSET="0x0"
READ_ONLY="no"
SHOW_CMP_FILES="no"
ARG_READ_AGAIN="no"
SDB_GSI_VENDOR_ID="0x651"
SDB_GSI_DEVICE_ID="0x169edcb8"
RAM_COUNT=1
RAM_OFFSET="0x02000000"
READ_FILE_NAME_BASE="cr_get_file_"
READ_FILE_NAME_BASE_READ_AGAIN="cr_get_file_ra_"
WRITE_FILE_NAME_BASE="cr_put_file_"
VERBOSE_MODE=0
PSRAM_ALL_ADDR_ARRAY=()
USE_DIFF="no"

# Run test
function run_test() {
  # Search RAM
  echo "Searching for PSRAM slave at $DEVICE ..."
  PSRAM_ADDR_BASE=$(eb-find "$DEVICE" "$SDB_GSI_VENDOR_ID" "$SDB_GSI_DEVICE_ID")
  if [ -z "$PSRAM_ADDR_BASE" ]; then
    echo "Error: PSRAM address base could not be determined."
    exit 1
  fi
  echo "Found PSRAM at $PSRAM_ADDR_BASE ..."

  # Interrate each RAM
  for CURRENT_RAM_ID in $(seq 1 $RAM_COUNT); do
    # Handle file name
    READ_FILE_NAME="$READ_FILE_NAME_BASE$CURRENT_RAM_ID"
    WRITE_FILE_NAME="$WRITE_FILE_NAME_BASE$CURRENT_RAM_ID"

    # Add argument offset, remove "0x"
    PSRAM_ADDR_BASE_CLEAN=${PSRAM_ADDR_BASE#0x}
    ARG_START_OFFSET_CLEAN=${ARG_START_OFFSET#0x}
    PSRAM_OFFSET_CLEAN=${RAM_OFFSET#0x}

    # Calculate offset based on RAM ID
    OFFSET_DEC=$((CURRENT_RAM_ID - 1))
    OFFSET_HEX=$(printf "%X" "$OFFSET_DEC")
    PSRAM_OFFSET_ID=$(echo "obase=16; ibase=16; $OFFSET_HEX * $PSRAM_OFFSET_CLEAN" | bc)
    PSRAM_OFFSET_ID_CLEAN=${PSRAM_OFFSET_ID#0x}
    PSRAM_OFFSET_ADDR_HEX=$(printf "0x%X" $PSRAM_OFFSET_ID_CLEAN)

    # Calculate complete start adress
    PSRAM_ADDR_OFFSET_COMPLETE=$(echo "ibase=16; $PSRAM_ADDR_BASE_CLEAN + $ARG_START_OFFSET_CLEAN + $PSRAM_OFFSET_ID_CLEAN" | bc)
    PSRAM_ADDR_HEX=$(printf "0x%X" $PSRAM_ADDR_OFFSET_COMPLETE)
    PSRAM_ALL_ADDR_ARRAY+=($PSRAM_ADDR_HEX)
    if [ $VERBOSE_MODE -eq 1 ]; then
      echo "Debug: PSRAM_ADDR_OFFSET_COMPLETE = $PSRAM_ADDR_OFFSET_COMPLETE"
      echo "Debug: PSRAM_ADDR_BASE_CLEAN = $PSRAM_ADDR_BASE_CLEAN"
      echo "Debug: ARG_START_OFFSET_CLEAN = $ARG_START_OFFSET_CLEAN"
      echo "Debug: PSRAM_OFFSET_ID_CLEAN = $PSRAM_OFFSET_ID_CLEAN"
      echo "Debug: PSRAM_OFFSET_ID = $PSRAM_OFFSET_ID"
      echo "Debug: PSRAM_ADDR_BASE = $PSRAM_ADDR_BASE"
      echo "Debug: PSRAM_ALL_ADDR_ARRAY = ${PSRAM_ALL_ADDR_ARRAY[@]}"
    fi

    # Start
    echo "Testing RAM ID $CURRENT_RAM_ID ..."
    echo "Using PSRAM address $PSRAM_ADDR_HEX ..."

    if [ "$READ_ONLY" = "no" ]; then
      echo "Creating test file ..."
      if [ "$FILE_WITH_ZEROS" = "no" ]; then
        echo "Using random data ..."
        dd if=/dev/urandom of=$WRITE_FILE_NAME bs=$BYTE_SIZE count=1
      else
        echo "Using zeros ..."
        dd if=/dev/zero of=$WRITE_FILE_NAME bs=$BYTE_SIZE count=1
      fi

      echo "Writing to PSRAM ..."
      t_start_w=$(date +%s.%N)
      eb-put "$DEVICE" "$PSRAM_ADDR_HEX" "$WRITE_FILE_NAME"
      t_end_w=$(date +%s.%N)
      t_duration_w=$(echo "($t_end_w - $t_start_w) * 1000" | bc)
      printf "Writing took %.3f ms ...\n" $t_duration_w
      sleep $WAIT_SECONDS
    fi

    echo "Reading (back) from PSRAM ..."
    t_start_r=$(date +%s.%N)
    eb-get "$DEVICE" "$PSRAM_ADDR_HEX/$BYTE_SIZE" "$READ_FILE_NAME"
    t_end_r=$(date +%s.%N)
    t_duration_r=$(echo "($t_end_r - $t_start_r) * 1000" | bc)
    printf "Reading took %.3f ms ...\n" $t_duration_r

    if [ "$READ_ONLY" = "no" ]; then
      echo "Comparing files ..."
      if cmp -s $WRITE_FILE_NAME $READ_FILE_NAME; then
        echo "Test passed ($WRITE_FILE_NAME == $READ_FILE_NAME)!"
      else
        echo "Test failed: Files are not identical."
        if [ "$USE_DIFF" = "yes" ]; then
          echo "Preparing debug output ..."
          colordiff <(xxd $WRITE_FILE_NAME) <(xxd $READ_FILE_NAME) -y --suppress-common-lines
        fi
        exit 1
      fi
    fi

    if [ "$SHOW_CMP_FILES" = "yes" ]; then
      echo "Written data:"
      hexdump -C $WRITE_FILE_NAME -v
      echo "Read data:"
      hexdump -C $READ_FILE_NAME -v
    fi
  done # for loop

  # Read all RAMs again
  if [ "$ARG_READ_AGAIN" = "yes" ]; then
    echo "Reading all RAMs again ..."
    for CURRENT_RAM_ID in $(seq 1 $RAM_COUNT); do
      CURRENT_RAM_ID_ALIGNED=$((CURRENT_RAM_ID - 1))
      READ_CURRENT_ADDRESS=${PSRAM_ALL_ADDR_ARRAY[$CURRENT_RAM_ID_ALIGNED]}
      READ_FILE_NAME_RA="$READ_FILE_NAME_BASE_READ_AGAIN$CURRENT_RAM_ID"
      echo "Using PSRAM address $READ_CURRENT_ADDRESS ..."
      eb-get "$DEVICE" "$READ_CURRENT_ADDRESS/$BYTE_SIZE" "$READ_FILE_NAME_RA"

      if [ "$READ_ONLY" = "no" ]; then
        WRITE_FILE_NAME="$WRITE_FILE_NAME_BASE$CURRENT_RAM_ID"
        echo "Comparing files ..."
        if cmp -s $WRITE_FILE_NAME $READ_FILE_NAME_RA; then
          echo "Test passed ($WRITE_FILE_NAME == $READ_FILE_NAME_RA)!"
        else
          echo "Test failed: Files are not identical."
          if [ "$USE_DIFF" = "yes" ]; then
            echo "Preparing debug output ..."
            colordiff <(xxd $WRITE_FILE_NAME) <(xxd $READ_FILE_NAME_RA) -y --suppress-common-lines
          fi
          exit 1
        fi
      fi
    done # for loop;
  fi
}

# Print Help
function print_help() {
  echo "Cellular RAM test application"
  echo ""
  echo "Usage:"
  echo "  $0 [options]"
  echo ""
  echo "Options:"
  echo "  -d <device>           - select device (dev/wbm0, dev/ttyUSB0, ...)"
  echo "  -b <bytes>            - write and read <bytes> from RAM"
  echo "  -w <seconds>          - wait between write and read"
  echo "  -o <offset>           - write and read beginning at <offset> (0x0, 0x4, ...)"
  echo "  -c <count>            - RAM chip(s) to test (1, 2, 3, 4)"
  echo "  -a                    - read all RAMs again (after write and read)"
  echo "  -z                    - write only zeros"
  echo "  -r                    - read only, don't write data to RAM"
  echo "  -s                    - show comparison files"
  echo "  -v                    - vebose mode, show additional debug information"
  echo "  -y                    - use diff in case of errors"
  echo "  -h                    - print help"
}

# Check given arguments
if [ $# -eq 0 ]; then
  echo "Error: Missing arguments!"
  print_help
  exit 1
fi

while getopts "d:b:w:o:c:rszavyh" opt; do
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
    o)
      ARG_START_OFFSET="$OPTARG"
      ;;
    c)
      RAM_COUNT="$OPTARG"
      ;;
    z)
      FILE_WITH_ZEROS="yes"
      ;;
    r)
      READ_ONLY="yes"
      ;;
    s)
      SHOW_CMP_FILES="yes"
      ;;
    a)
      ARG_READ_AGAIN="yes"
      ;;
    v)
      VERBOSE_MODE=1
      ;;
    y)
      USE_DIFF="yes"
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
REQUIRED_TOOLS=("eb-find" "eb-put" "eb-get" "dd" "bc" "hexdump")
if [ "$USE_DIFF" = "yes" ]; then
  REQUIRED_TOOLS+=("diff" "xxd")
fi

# Check for the presence of each tool in the array
for TOOL in "${REQUIRED_TOOLS[@]}"; do
  if ! command -v "$TOOL" &>/dev/null; then
    echo "Error: Required tool '$TOOL' not found. Make sure it is installed and in the PATH."
    exit 1
  fi
done

run_test

exit 0
