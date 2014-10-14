#!/bin/sh
# ====================================================================================================
################################################################################
# @file activation.sh
# @brief Commissioning and activation for new boards (vetar2a, ...)
#
# Copyright (C) 2014 GSI Helmholtz Centre for Heavy Ion Research GmbH 
#
# @author C. Prados <c.prados@gsi.de>
# @author A. Hahn <a.hahn@gsi.de>
#
# @bug No known bugs.
#
################################################################################
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#  
# You should have received a copy of the GNU Lesser General Public
# License along with this library. If not, see <http://www.gnu.org/licenses/>.
################################################################################

# Environment setup (binaries, variables and paths)
# ====================================================================================================
VETAR2A_SOF_FILE=../../syn/gsi_vetar2a/wr_core_demo/vetar2a.sof # See func_check_environment
VETAR2A_RPD_FILE=../../syn/gsi_vetar2a/wr_core_demo/vetar2a.rpd # ...
USB_FLASHER=../../ip_cores/etherbone-core/hdl/eb_usb_core       # ...
OW_WRITE=../../ip_cores/wrpc-sw/tools/eb-w1-write               # ...
WRPC_BIN=../../ip_cores/wrpc-sw/tools/sdb-wrpc.bin              # ...
BASE_DIR=$PWD                                                   # Working directory
USB_DEVICE=$1  # Argument 1 (USB device)
HARDWARE=$2    # Argument 2 (hardware target)
QUARTUS_DIR=$3 # Argument 3 (Quartus base directory)
CONTINUE=;     # Read user input (continue ...)
USB_LINK=;     # Contains USB link status
USB_HWDET=;    # USB hardware detected?
SKIP=;         # Skip section/test
RET=;          # Contains return value of the last system call

# Quartus
# ====================================================================================================
QUARTUS_BASE=$QUARTUS
QUARTUS_BIN=$QUARTUS_BASE/bin

# User setup (only change this if you really know what you are doing)
# ====================================================================================================
MAC_ADDRESS_PATTERN="02:ff:00:02:00:XX"
IO_TEST_STEP1="yes"
IO_TEST_STEP2="yes"
IO_TEST_STEP3="yes"
PROGRAM_FPGA="yes"
ADDON_JTAG="yes"
FLASH_FPGA="yes"
FORMAT_OW="yes"
FLASH_USB="yes"
CHECK_GUI="yes"
SET_MAC="yes"

# Function check_usb_connection()
# - Checks the USB connection
# ====================================================================================================
func_check_usb_connection()
{
  # Wait until device is up
  USB_LINK=0
  USB_HWDET=0
  while [ $USB_HWDET -ne 1 ]
  do
    lsusb | grep "OpenMoko"
    RET=$?
    if [ $RET -ne 1 ]
    then
      USB_HWDET=1
      while [ $USB_LINK = 0 ]
      do
        # Little hack, using return code from eb-ls #TBD: Evaluate "time-out"?
        eb-ls $USB_DEVICE > /dev/null 2>&1
        RET=$?
        if [ $RET -ne 1 ]
        then
          echo "USB link established ($USB_DEVICE)!"
          USB_LINK=1
        else
          echo "Waiting for USB link ($USB_DEVICE)..."
          USB_LINK=0
        fi
        sleep 1
      done
    else
      echo "Missing OpenMoko device ..."
    fi
    sleep 1
  done
}

# Function func_continue_or_skip()
# - Skips or continues next step
# ====================================================================================================
func_continue_or_skip()
{
  CONTINUE=;
  SKIP=;
  while [ -z "$CONTINUE" ] 
  do
    read -r -p "Type anything but \"c\" to continue or \"s\" to skip. [c/s]: " CONTINUE
  done
  if [ $CONTINUE = s ]
  then
    SKIP=y
  else
    SKIP=;
  fi
}

# Function func_check_environment(...)
# - Checks files and binaries
# ====================================================================================================
func_check_environment()
{
  echo "\nCheck for dependencies/necessary files"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  # Check Quartus directory
  if [ -d "$QUARTUS_BASE" ]
  then
    echo "Found Quartus directory at: $QUARTUS_BASE"
  else
    # Take Quartus base directory from argument
    echo "Note: Missing Quartus, using path from argument ..."
    QUARTUS_BASE=$QUARTUS_DIR
    if [ -d "$QUARTUS_BASE" ]
    then
      echo "Found Quartus directory at: $QUARTUS_BASE"
    else
      echo "Error: Missing Quartus base directory ($QUARTUS_BASE)!"
      exit 1
    fi
  fi
  # Check RPD file
  if [ -f "$VETAR2A_RPD_FILE" ]
  then
    echo -n "Using RPD file: "
    file $VETAR2A_RPD_FILE
  else
    echo "Error: RPD file is missing!"  
    exit 1
  fi
  # Check SOF file
  if [ -f "$VETAR2A_SOF_FILE" ]
  then
    echo -n "Using SOF file: "
    file $VETAR2A_SOF_FILE
  else
    echo "Error: SOF file is missing!"  
    exit 1
  fi
  # Check USB flash script
  if [ -f "$BASE_DIR/$USB_FLASHER/flash-fx2lp.sh" ]
  then
    echo -n "Using USB flash script: "
    file $USB_FLASHER/flash-fx2lp.sh
  else
    echo "Error: USB flash script is missing!"  
    exit 1
  fi
  # Check OneWire write script
  if [ -f "$OW_WRITE" ]
  then
    echo -n "Using OneWire write script: "
    file $OW_WRITE
  else
    echo "Error: OneWire write script is missing!"  
    exit 1
  fi
  # Check WRPC binary
  if [ -f "$WRPC_BIN" ]
  then
    echo -n "Using WRPC sdb binary: "
    file $OW_WRITE
  else
    echo "Error: WRPC sdb binary is missing!"  
    exit 1
  fi
  echo "\nAll files are present!\n"
  echo "Compiling the device test ..."
  make device-test-$HARDWARE
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: Can't compile device test!"
    exit 1
  fi
  echo "\nPlease turn off the complete board, attach the WRPX1 and the addon board."
  echo "Warning: DISCONNECT ALL OTHER DEVICES FROM USB (LIKE EXPLODERx, VETARx, ...)!"
}

# Function func_program_fpga(...)
# - Programs the FPGA
# - Parameter $1:
# - => 0      = Skip USB connection test
# - => 1/else = Do USB connection test
# - Parameter $2:
# - => 0      = Use base board JTAG
# - => 1/else = Use addon board JTAG
# ====================================================================================================
func_program_fpga()
{
  echo "\nCheck JTAG connection and program FPGA"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $FLASH_USB = "yes" ]
  then
    if [ $2 -eq 0 ]
    then
      echo "Please connect the JTAG connector to the BASE BOARD."
    else
      echo "Please do a power-cycle."
      echo "Please connect the JTAG connector to the ADDON BOARD."
    fi
    func_continue_or_skip
    if [ -z "$SKIP" ] 
    then
      if [ $1 -eq 0 ]
      then
        echo "Skipping USB connection test ..."
      else
        func_check_usb_connection
      fi
      echo "Programming the Vetar2a"
      $QUARTUS_BIN/quartus_pgm -c 1 -m jtag -o "p;$VETAR2A_SOF_FILE" 
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: Flash attempt failed!"
        exit 1
      fi
    else
      echo "Skipping this step ..."
    fi
  fi
}

# Function func_flash_usb_device(...)
# - Flashed the USB chip/device
# ====================================================================================================
func_flash_usb_device()
{
  echo "\nFlash USB device"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $FLASH_USB = "yes" ]
  then
    echo "JTAG should be connected."
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      echo "Trying to erase the USB device now..."
      cd $BASE_DIR/$USB_FLASHER
      $BASE_DIR/$USB_FLASHER/flash-fx2lp.sh -E
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: Flash attempt failed!"
        exit 1
      fi
      echo "Trying to flash the USB device now..."
      $BASE_DIR/$USB_FLASHER/flash-fx2lp.sh
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: Flash attempt failed!"
        exit 1
      fi
      cd $BASE_DIR
      echo "USB device is flashed now!"
    else
      echo "Skipping this step ..."
    fi
  fi
}

# Function func_flash_fpga(...)
# - Used to flash the FPGA (writes RPD file to SPI flash)
# ====================================================================================================
func_flash_fpga()
{
  echo "\nFlash FPGA"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $FLASH_FPGA = "yes" ]
  then
    echo "USB should be connected."
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      echo "Trying to flash the FPGA now ..."
      eb-flash $USB_DEVICE $VETAR2A_RPD_FILE
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: Flash attempt failed!"
        exit 1
      fi
      echo "FPGA is flashed now!"
    else
      echo "Skipping this step ..."
    fi
  fi
}

# Function func_format_onewire(...)
# - This function is used to format the OneWire EEPROM
# - The EEPROM is needed to store the MAC address. Necessary for function func_set_mac_address.
# ====================================================================================================
func_format_onewire()
{
  echo "\nFormat the 1-wire EEPROM"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $FORMAT_OW = "yes" ]
  then
    echo "Please do a power cycle"
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      echo "Formating the 1-wire EEPROM ..."
      $OW_WRITE $USB_DEVICE 0 320 < $WRPC_BIN
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: Formating attempt failed!"
        exit 1
      fi
      echo "1-wire EEPROM is formated now!"
    else
      echo "Skipping this step ..."
    fi
  fi
}

# Function func_io_connection_test (...)
# - This is an IO connection test with 3 steps
# - At each step the must change or remove some cables and wires
# ====================================================================================================
func_io_connection_test()
{
  echo "\nTest I/Os"
  # Step 1
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $IO_TEST_STEP1 = "yes" ]
  then
    echo "Please connect the following I/Os:"
    echo "- OUT1 <=> IO1 (lemo)"
    echo "- OUT2 <=> IO2 (lemo)"
    echo "- OUT3 <=> IO3 (lemo)"
    echo "- OUT  <=> IN (lemo - near SFP cage)"
    echo "- I1   <=> O1 (LVDS box header)"
    echo "- I2   <=> O2 (LVDS box header)"
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      ./device-test-$HARDWARE $USB_DEVICE testcase1
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: I/O test failed!"
        exit 1
      fi
    else
      echo "Skipping step #1 ..."
    fi
  fi
  echo "\n"
  # Step 2
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $IO_TEST_STEP2 = "yes" ]
  then
    echo "Please connect the following I/Os:"
    echo "- IO1  <=> IN1 (lemo)"
    echo "- IO2  <=> IN2 (lemo)"
    echo "- IO3  <=> IN (lemo - near SFP cag)"
    echo "- HDMI <=> HDMI"
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      ./device-test-$HARDWARE $USB_DEVICE testcase2
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: I/O test failed!"
        exit 1
      fi
    else
      echo "Skipping step #2 ..."
    fi
  fi
  echo "\n"
  # Step 3
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $IO_TEST_STEP3 = "yes" ]
  then
    echo "Please disconnect all LEMOs, LVDS-connections and HDMI cable."
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      ./device-test-$HARDWARE $USB_DEVICE testcase3
      RET=$?
      if [ $RET -ne 0 ]
      then
        echo "Error: I/O test failed!"
        exit 1
      fi
    else
      echo "Skipping step #3 ..."
    fi
  fi
  echo "\n"
  # Done
  echo "I/O test finished successfully!"
}

# Function func_set_mac_address(...)
# - MAC address should be set by user here (according to the given MAC pattern)
# ====================================================================================================
func_set_mac_address()
{
  echo "\nSet MAC address"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $SET_MAC = "yes" ]
  then
    echo "Setting MAC address now to $MAC_ADDRESS_PATTERN"
    echo "- Please make sure that the USB cable is connected to the base board"
    echo "- Attach a SFP to the base board"
    echo "- Use the command \"mac setp $MAC_ADDRESS_PATTERN"
    echo "- After setting the MAC address press ctrl+c" 
    func_continue_or_skip
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      eb-console $USB_DEVICE
      echo "MAC address set!"
      func_check_wr_link
    else
      echo "Skipping this step ..."
    fi
  fi
}

# Function func_check_wr_link(...)
# - Link to WR network should be checked by user here
# ====================================================================================================
func_check_wr_link()
{
  echo "\nCheck WR LINK"
  echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  if [ $CHECK_GUI = "yes" ]
  then
    echo "Check WR link with"
    echo "- Please make sure that the USB cable is connected to the base board"
    echo "- Do a power cycle"
    echo "- Use the command \"gui\""
    echo "- After setting the MAC address press ctrl+c" 
    if [ -z "$SKIP" ]
    then
      func_check_usb_connection
      eb-console $USB_DEVICE
      echo "WR LINK IS UP"
    else
      echo "Skipping this step ..."
    fi
  fi
}

# Script starts here
# ====================================================================================================
# Welcome message
echo "Commissioning script started ..."
# Expecting two arguments
if [ 2 -ne $# ] && [ 3 -ne $# ]
then
  echo "Error: This script needs exactly 2 or 3 parameters:"
  echo "- Parameter #1: Device"
  echo "- Parameter #2: Hardware target"
  echo "- Parameter #3: Quartus base directory (optional)"
  echo "Example usage: ./activation.sh dev/ttyUSB0 vetar2a"
  echo "Example usage (optional): ./activation.sh dev/ttyUSB0 vetar2a /bin/quartus/"
  echo ""
  echo "JTAG Setup Vetar2a"
  echo "++++++++++++++++++"
  echo "PROMO5 configuration: 0x1 0x4 (recommended JTAG adapter)"
  echo "PROMO3 configuration: 0x3"
  echo ""
  echo "Trouble shooting"
  echo "++++++++++++++++"
  echo "Error (213019): Can't scan JTAG chain. Error code 87: Close the Quartus programmer and SignalTap"
  echo "Permission denied: /dev/bus/usb/0NN/0XX: Get the rights for this device (sudo chmod 777 /dev/bus/usb/0NN/0XX) or use sudo (i.e. Ubuntu)"
  exit 1
fi
# Show parameters
echo "Device: $USB_DEVICE"
echo "Hardware target: $HARDWARE"
if [ $QUARTUS_DIR ]
then
  echo "Optional Quartus directory: $QUARTUS_DIR"
fi
# Run functions
func_check_environment
func_program_fpga 0 0
func_flash_usb_device
func_program_fpga 0 1
func_flash_fpga
func_format_onewire
func_io_connection_test
func_set_mac_address
# Done
echo "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "Commissioning script finished successfully!\n"
exit 0
