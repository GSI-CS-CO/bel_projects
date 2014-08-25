#!/bin/sh

# Environment setup
# ====================================================================================================
QUARTUS=/home/alex/workspace/optional/quartus/altera/quartus
QUARTUS_BIN=$QUARTUS/bin
VETAR2A_SOF_FILE=../../syn/gsi_vetar2a/wr_core_demo/vetar2a.sof
VETAR2A_RPD_FILE=../../syn/gsi_vetar2a/wr_core_demo/vetar2a.rpd
USB_FLASHER=/../../ip_cores/etherbone-core/hdl/eb_usb_core
OW_WRITE=../../ip_cores/wrpc-sw/tools/eb-w1-write
WRPC_BIN=../../ip_cores/wrpc-sw/tools/sdb-wrpc.bin
BASE_DIR=$PWD # Working directory
USB_DEVICE=$1 # Argument 1
HARDWARE=$2   # Argument 2
CONTINUE=;
USB_LINK=;

# User setup
# ====================================================================================================
FLASH_USB="no"
FLASH_FPGA="yes"
FORMAT_OW="no"
ADDON_JTAG="no"
IO_TEST_STEP1="no"
IO_TEST_STEP2="no"
SET_MAC="no"

# Script start
# ====================================================================================================
echo "Commissioning script started ..."
if [ 2 -ne $# ] # Expecting two arguments
then
  echo "Error: This script needs exactly 2 parameters:"
  echo "- Parameter #1: Device"
  echo "- Parameter #2: Hardware target"
  echo "Example usage: ./activation.sh dev/ttyUSB0 vetar2a"
  exit 1
fi

# Check files and binaries
# ====================================================================================================
echo "\nStep 1: Checking dependencies/necessary files";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ -f "$VETAR2A_RPD_FILE" ]
then
  echo -n "Using RPD file: "
  file $VETAR2A_RPD_FILE
else
  echo "Error: RPD file is missing!"  
  exit 1
fi
if [ -f "$VETAR2A_SOF_FILE" ]
then
  echo -n "Using SOF file: "
  file $VETAR2A_SOF_FILE
else
  echo "Error: SOF file is missing!"  
  exit 1
fi
if [ -f "$BASE_DIR/$USB_FLASHER/flash-fx2lp.sh" ]
then
  echo -n "Using USB flash script: "
  file $BASE_DIR/$USB_FLASHER/flash-fx2lp.sh
else
  echo "Error: USB flash script is missing!"  
  exit 1
fi
echo "All files are present!\n"
echo "Compiling the device test ..."
make device-test
echo "\nPlease turn off the complete board, attach the WRPX1 and the addon board and do a power cycle";
CONTINUE=;
while [ -z "$CONTINUE" ]; do
  read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
done;

# Flash USB device
# ====================================================================================================
echo "\nStep 2: Flashing USB device";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FLASH_USB = "yes" ]
then
  echo "Please connect the USB connector to the base board.";
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  echo "Trying to flash the USB device now..."
  cd $BASE_DIR/$USB_FLASHER;
  $BASE_DIR/$USB_FLASHER/flash-fx2lp.sh -E;
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: Flash attempt failed!"
    exit 1;
  fi
  $BASE_DIR/$USB_FLASHER/flash-fx2lp.sh;
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: Flash attempt failed!"
    exit 1;
  fi
  cd $BASE_DIR;
  echo "USB device is flashed now!"
else
  echo "Skipping this step ..."
fi

# Flash FPGA
# ====================================================================================================
echo "\nStep 3: Flashing FPGA";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FLASH_FPGA = "yes" ]
then
  echo "Please connect the JTAG connector to the base board and reset it (power cycle).";
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  echo "Trying to flash the FPGA now ..."
  eb-flash $USB_DEVICE $VETAR2A_RPD_FILE
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: Flash attempt failed!"
    exit 1;
  fi
  echo "FPGA is flashed now!"
else
  echo "Skipping this step ..."
fi

# Format 1-wire
# ====================================================================================================
echo "\nStep 4: Format the 1-wire EEPROM";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FORMAT_OW = "yes" ]
then
  echo "Please do a power cycle";
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  echo "Formating the 1-wire EEPROM ..."
  $OW_WRITE $USB_DEVICE 0 320 < $WRPC_BIN
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: Formating attempt failed!"
    exit 1;
  fi
  echo "1-wire EEPROM is formated now!"
else
  echo "Skipping this step ..."
fi

# JTAG addon test
# ====================================================================================================
echo "\nStep 5: Check JTAG connection from addon board";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FLASH_USB = "yes" ]
then
  echo "Please connect the JTAG connector to the addon board and reset it (power cycle).";
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  echo "Programming the Vetar2a"
  $QUARTUS_BIN/quartus_pgm -c 1 -m jtag -o "p;$VETAR2A_SOF_FILE" 
else
  echo "Skipping this step ..."
fi

# IO Connection test
# ====================================================================================================
echo "\nStep 6: Test I/Os";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $IO_TEST_STEP1 = "yes" ]
then
  echo "Please connect the following I/Os:";
  echo "- OUT1 <=> IO1 (lemo)";
  echo "- OUT2 <=> IO2 (lemo)";
  echo "- OUT3 <=> IO3 (lemo)";
  echo "- OUT  <=> IN (lemo - near SFP cage)";
  echo "- I1   <=> O1 (LVDS box header)";
  echo "- I2   <=> O2 (LVDS box header)";
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  ./device-test $USB_DEVICE testcase1
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: I/O test failed!"
    exit 1;
  fi
else
  echo "Skipping step #1 ..."
fi

if [ $IO_TEST_STEP2 = "yes" ]
then
  echo "Please connect the following I/Os:";
  echo "- IO1  <=> IN1 (lemo)";
  echo "- IO2  <=> IN2 (lemo)";
  echo "- IO3  <=> IN (lemo - near SFP cag)";
  echo "- HDMI <=> HDMI";
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  ./device-test $USB_DEVICE testcase2
  RET=$?
  if [ $RET -ne 0 ]
  then
    echo "Error: I/O test failed!"
    exit 1;
  fi
else
  echo "Skipping step #2 ..."
fi

echo "I/O test finished successfully!"

# Set MAC address
# ====================================================================================================
echo "\nStep 7: Set MAC address";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $SET_MAC = "yes" ]
then
  echo "Setting MAC address now to 02:ff:00:02:00:XX";
  echo "- Please make sure that the USB cable is connected to the base board"
  echo "- Use the command \"mac setp 02:ff:00:02:00:XX "
  echo "- Attach a SFP to the base board"
  echo "- After setting the MAC address press ctrl+c" 
  CONTINUE=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; 
  done;
  eb-console $USB_DEVICE
else
  echo "Skipping this step ..."
fi

echo "MAC address set!"


# Finish test
# ====================================================================================================
echo "\nCommissioning script finished successfully!\n"
exit 0;
