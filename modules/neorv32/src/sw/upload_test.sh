#!/bin/bash
set -u

# Arguments
v_nrv_device="NULL"
v_nrv_program="NULL"
v_nrv_compare="NULL"
v_nrv_stop=false
v_nrv_start=false
v_nrv_restart=false
v_nrv_keep_reset=false
v_nrv_help=false
v_nrv_vebose=0
v_nrv_examine=false
v_nrv_preserve=false

# Settings
declare -r v_nrv_sdb_gsi_vendor_id="0x651"
declare -r v_nrv_sdb_gsi_ram_id="0x83075320"
declare -r v_nrv_sdb_gsi_rst_ctl_id="0x3a362063"
declare -r v_nrv_reset_offset="0x20"
declare -r v_nrv_reset_neorv32_stop="0x0"
declare -r v_nrv_reset_neorv32_start="0x1"
declare -r v_nrv_dpram_neorv32="0x71000000"
declare -r v_nrv_readback_neorv32="firmware-neorv32.bin"
declare -r v_nrv_sdb_gsi_fifo_id="0xcacacaca"

declare -r FIFO_SIZE=512

# Internal parameters
v_nrv_ram_addr="NULL"
v_nrv_reset_ctl_addr="NULL"
v_nrv_reset_ctl_neorv32_addr="NULL"
v_nrv_fifo_addr="NULL"
v_nrv_fifo_status_reg="NULL"
v_nrv_fifo_boot_reg="NULL"

# Functions
f_log() {
  local level="$1"
  local msg="$2"
  case "$level" in
    INFO)  [ $v_nrv_vebose -ge 1 ] && echo "Info: $msg" ;;
    ERROR) echo "Error: $msg" >&2 ;;
  esac
  return 0
}

is_fifo_full() {
  local status_val
  local FULL_MASK=0x2
  echo $v_nrv_fifo_status_reg
  status_val=0x$(eb-read "$v_nrv_device" "$v_nrv_fifo_status_reg"/4)
  echo $status_val
  if (((status_val & FULL_MASK) == 1)); then
    echo "return0"
    return 0
  else 
    return 1
  fi
}

function f_find_neorv32_ram() {
  v_nrv_ram_addr=$(eb-find "$v_nrv_device" "$v_nrv_sdb_gsi_vendor_id" "$v_nrv_sdb_gsi_ram_id")
  if [ $? -ne 0 ] || [ -z "$v_nrv_ram_addr" ]; then
    f_log ERROR "NEORV32 RAM address base could not be determined!"
    exit 10
  fi
  f_log INFO "Found NEORV32 RAM address $v_nrv_ram_addr ..."
}

function f_find_neorv32_fifo() {
  v_nrv_fifo_addr=$(eb-find "$v_nrv_device" "$v_nrv_sdb_gsi_vendor_id" "$v_nrv_sdb_gsi_fifo_id")
  if [ $? -ne 0 ] || [ -z "$v_nrv_fifo_addr" ]; then
    f_log ERROR "NEORV32 FIFO address base could not be determined!"
    exit 10
  fi
  v_nrv_fifo_status_reg=$(printf "0x%x" $((v_nrv_fifo_addr+4)))
  v_nrv_fifo_boot_reg=$(printf "0x%x" $((v_nrv_fifo_addr+8)))
  f_log INFO "Found NEORV32 FIFO address $v_nrv_fifo_addr ..."
  f_log INFO "Found NEORV32 FIFO Boot Register $v_nrv_fifo_boot_reg ..."
}

function f_find_reset_ctl() {
  v_nrv_reset_ctl_addr=$(eb-find "$v_nrv_device" "$v_nrv_sdb_gsi_vendor_id" "$v_nrv_sdb_gsi_rst_ctl_id")
  if [ $? -ne 0 ] || [ -z "$v_nrv_reset_ctl_addr" ]; then
    f_log ERROR "Reset controller address base could not be determined!"
    exit 2
  fi
  f_log INFO "Found reset controller address $v_nrv_reset_ctl_addr ..."
  v_nrv_reset_ctl_neorv32_addr=$(printf "0x%x" $(( $v_nrv_reset_ctl_addr + $v_nrv_reset_offset )))
  f_log INFO "Using NEORV32 reset address $v_nrv_reset_ctl_neorv32_addr ..."
}

function f_upload_program() {
  if [ "$v_nrv_program" != "NULL" ]; then
    if [ -f "$v_nrv_program" ]; then
      f_stop_neorv32
      f_start_neorv32
      echo "Restart Neorv32 ..."
      eb-write "$v_nrv_device" "$v_nrv_fifo_boot_reg"/4 "$v_nrv_reset_neorv32_start"     #0x01 to fifo
      echo "Access FIFO to load data ..."
      echo "Using file $v_nrv_program to program the CPU ..."
      FILE_SIZE=$(stat -c %s "$v_nrv_program")
      eb-write "$v_nrv_device" "$v_nrv_fifo_addr"/4 "$FILE_SIZE"      #first data to fifo is file size
      #rm -rf temp && mkdir temp
      #echo "after remove"
      #split -b "$FIFO_SIZE" -d -a 3 --additional-suffix=".bin" "$v_nrv_program" "temp/chunk_"
      word_idx=0
      for word in $(od -v -A n -t x4 "$v_nrv_program"); do
        # if fifo is not empty then wait for 5 ms
        while is_fifo_full; do
          sleep 0.005
          echo "full"
        done
        #f_log INFO "FIFO is empty now, Page Write: $chunk"
        echo "not full now"
        #eb-put -l -q "$v_nrv_device" "$v_nrv_fifo_addr" "$chunk"
        eb-write "$v_nrv_device" "$v_nrv_fifo_addr"/4 "0x$word"
        if [ $? -ne 0 ]; then
          f_log ERROR "Failed to upload program $v_nrv_program at word $word_idx!"
          exit 3
        fi
        echo "$word_idx"
        ((word_idx++))
      done
      echo "complete"
      f_log INFO "[SUCCESS] $v_nrv_program upload is complete."
    fi
  fi
}
  
function f_update_dpram() {
  if [ "$v_nrv_dpram" != "NULL" ]; then
    if [ -f "$v_nrv_dpram" ]; then
      f_stop_neorv32
      f_start_neorv32
      f_log INFO "Restart Neorv32 ..."
      eb-write "$v_nrv_device" "$v_nrv_fifo_boot_reg"/4 "$v_nrv_reset_neorv32_stop"     #0x00 to dpram
      f_log INFO "Access DPRAM to load data ..."
      f_log INFO "Using file $v_nrv_dpram to program the CPU ..."
      eb-put -l -q "$v_nrv_device" "$v_nrv_ram_addr" "$v_nrv_dpram"
      if [ $? -ne 0 ]; then
        f_log ERROR "Failed to upload program $v_nrv_dpram!"
        exit 3
      fi
      FILE_SIZE=$(stat -c %s "$v_nrv_dpram")
      eb-write "$v_nrv_device" "$v_nrv_fifo_addr"/4 "$FILE_SIZE"
      f_log INFO "[SUCCESS] Neorv32 DPRAM programmed with $v_nrv_program."
      if [ "$v_nrv_examine" == true ]; then
        f_log INFO "Verifying the RAM content against the program file..."
        v_nrv_program_size=$(wc -c < "$v_nrv_dpram")
        eb-get -l -q "$v_nrv_device" "$v_nrv_dpram_neorv32"/"$v_nrv_program_size" "$v_nrv_readback_neorv32"
        if [ $? -ne 0 ]; then
          f_log ERROR "Failed to readback firmware image from $v_nrv_device!"
          exit 8
        fi
        cmp "$v_nrv_readback_neorv32" "$v_nrv_program"
        v_nrv_cmp_status=$?
        if [ "$v_nrv_preserve" == true ]; then
          f_log INFO "Preserve the readback file $v_nrv_readback_neorv32 from RAM"
        else
          rm -f "$v_nrv_readback_neorv32"
        fi
        if [ $v_nrv_cmp_status -ne 0 ]; then
          f_log ERROR "Verification failed, the program file and RAM content mismatch!" 
        else
          f_log INFO "[SUCCESS] Program uploaded and verified."
        fi
        [ $v_nrv_cmp_status -ne 0 ] && exit 9
       
      fi   
    else
      f_log ERROR "Can't find file $v_nrv_program!"
      exit 4
    fi
  else
    f_ctl_mode
  fi
}

function f_compare_program(){
  if [ "$v_nrv_compare" != "NULL" ]; then
    if [ -f "$v_nrv_compare" ]; then
      f_log INFO "Comparing the file $v_nrv_compare with the RAM content..."
      v_nrv_program_size=$(wc -c < "$v_nrv_compare")
      eb-get -l -q "$v_nrv_device" "$v_nrv_dpram_neorv32"/"$v_nrv_program_size" "$v_nrv_readback_neorv32"
      if [ $? -ne 0 ]; then
        f_log ERROR "Failed to readback firmware image from $v_nrv_device!"
        exit 13
      fi
      cmp "$v_nrv_readback_neorv32" "$v_nrv_compare"
      v_nrv_cmp_status=$?
      if [ "$v_nrv_preserve" == true ]; then
        f_log INFO "Preserve the readback file $v_nrv_readback_neorv32 from RAM"
      else
        rm -f "$v_nrv_readback_neorv32"
      fi
      if [ $v_nrv_cmp_status -ne 0 ]; then
        f_log ERROR "The file and RAM content mismatch!" 
      else
        f_log INFO "[SUCCESS] The file and RAM content match."
      fi
      [ $v_nrv_cmp_status -ne 0 ] && exit 14
    else 
      f_log ERROR "Cannot find file $v_nrv_compare!"
      exit 15
    fi
  fi
}

function f_stop_neorv32() {
  f_log INFO "Stopping NEORV32 ..."
  eb-write "$v_nrv_device" "$v_nrv_reset_ctl_neorv32_addr"/4 "$v_nrv_reset_neorv32_stop"
  if [ $? -ne 0 ]; then
    f_log ERROR "Failed to stop NEORV32!"
    exit 5
  fi
}

function f_start_neorv32() {
  f_log INFO "Starting NEORV32 ..."
  eb-write "$v_nrv_device" "$v_nrv_reset_ctl_neorv32_addr"/4 "$v_nrv_reset_neorv32_start"
  if [ $? -ne 0 ]; then
    f_log ERROR "Failed to start NEORV32!"
    exit 6
  fi
}

function f_ctl_mode() {
  # Validate mutually exclusive options
  if [[ "$v_nrv_start" == true && "$v_nrv_stop" == true ]] || \
     [[ "$v_nrv_start" == true && "$v_nrv_restart" == true ]] || \
     [[ "$v_nrv_stop" == true && "$v_nrv_restart" == true ]]; then
    f_log ERROR "Options -s, -t, and -r are mutually exclusive!" >&2
    exit 7
  fi
  if [ "$v_nrv_start" = true ]; then
    f_start_neorv32
  fi
  if [ "$v_nrv_stop" = true ]; then
    f_stop_neorv32
  fi
  if [ "$v_nrv_restart" = true ]; then
    f_stop_neorv32
    f_start_neorv32
  fi
}

function f_print_help() {
  cat << EOF
Usage: $(basename "$0") [OPTIONS] DEVICE

Control and program NEORV32 CPU.

Options:
  -f FILE      Use fifo to upload Program file
  -d FILE      Upload Program file to DPRAM
  -c FILE      Compare file with the RAM content
  -k           Keep CPU in reset after programming 
  -t           Stop the CPU
  -s           Start the CPU
  -r           Restart the CPU
  -v           Enable verbose output
  -e           Verify the RAM content after programming (only works with -f FILE)
  -p           Preserve the downloaded RAM content in readback file
  -h           Show this help message

Example:
  Program NEORV32                                      : $(basename "$0") -v -f program.bin dev/ttyUSB0
  Program NEORV32 and verify                           : $(basename "$0") dev/ttyUSB0 -v -f program.bin -e
  Preserve the downloaded RAM content after verifying  : $(basename "$0") dev/ttyUSB0 -v -f program.bin -e -p
  Compare file with NEORV32 RAM content                : $(basename "$0") dev/ttyUSB0 -v -c sample.bin
  Preserve the downloaded RAM content after comparing  : $(basename "$0") dev/ttyUSB0 -v -c sample.bin -p
EOF
}

# Process arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    -k)
      v_nrv_keep_reset=true
      shift
      ;;
    -t)
      v_nrv_stop=true
      shift
      ;;
    -s)
      v_nrv_start=true
      shift
      ;;
    -r)
      v_nrv_restart=true
      shift
      ;;
    -h)
      v_nrv_help=true
      shift
      ;;
    -f)
      v_nrv_program=$2
      shift 2
      ;;
    -d)
      v_nrv_dpram=$2
      shift 2
      ;;
    -v)
      v_nrv_vebose=1
      shift
      ;;
    -c)
      if [ $# -lt 2 ] || [[ "$2" == -* ]]; then
        f_log ERROR "Option -c requires a file argument."
        exit 16
      fi
      v_nrv_compare=$2
      shift 2
      ;;
    -e)            
      v_nrv_examine=true
      shift
      ;;   
    -p)
      v_nrv_preserve=true
      shift
      ;;
    -*)
      f_log ERROR "Unknown argument: $1"
      exit 11
      ;;
    *)
      if [ "$v_nrv_device" == "NULL" ];then
        v_nrv_device="$1"
        shift
      else
        f_log ERROR "Too many input argument: $1 (Already using device $v_nrv_device)"
        exit 12
      fi
      ;;
    esac
done

if [ "$v_nrv_help" = true ]; then
  f_print_help
  exit 0
else
  # Find device
  f_find_neorv32_ram
  f_find_reset_ctl
  f_find_neorv32_fifo

  # Program and control CPU
  f_upload_program
  f_update_dpram
  # Compare the file from user with the RAM content
  f_compare_program 
fi
exit 0
