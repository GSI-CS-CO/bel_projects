#!/bin/sh

# Environment setup
# ====================================================================================================
QUARTUS=/home/alex/workspace/optional/quartus/altera/quartus
QUARTUS_BIN=$QUARTUS/bin
VETAR2A_SOF_FILE=../../syn/gsi_vetar2a/wr_core_demo/vetar2a.sof
VETAR2A_RPD_FILE=../../syn/gsi_vetar2a/wr_core_demo/vetar2a.rpd
USB_FLASHER=../../ip_cores/etherbone-core/hdl/eb_usb_core
OW_WRITE=../../ip_cores/wrpc-sw/tools/eb-w1-write
WRPC_BIN=../../ip_cores/wrpc-sw/tools/sdb-wrpc.bin
BASE_DIR=$PWD # Working directory
USB_DEVICE=$1 # Argument 1
HARDWARE=$2   # Argument 2
CONTINUE=;    # Read user input (continue ...)
USB_LINK=;    # Contains USB link status
USB_HWDET=;   # USB hardware detected?
SKIP=;        # Skip section/test
RET=;         # Contains return value of the latest system call

# User setup
# ====================================================================================================
FLASH_USB="yes"
FLASH_FPGA="yes"
FORMAT_OW="yes"
ADDON_JTAG="yes"
IO_TEST_STEP1="yes"
IO_TEST_STEP2="yes"
SET_MAC="yes"
CHECK_GUI="yes"

# Function check_usb_connection() - Checks USB connection
# ====================================================================================================
check_usb_connection()
{
  # Wait until device is up
  USB_LINK=0;
  USB_HWDET=0;
  
  while [ $USB_HWDET -ne 1 ]; do
    lsusb | grep "OpenMoko";
    RET=$?
    if [ $RET -ne 1 ]; then
      USB_HWDET=1;
      while [ $USB_LINK = 0 ]; do
        eb-ls $USB_DEVICE > /dev/null 2>&1; # Little hack, using return code for eb-ls #TBD: Evaluate "time-out"?
        RET=$?
        if [ $RET -ne 1 ]; then
          echo "USB link established ($USB_DEVICE)!"
          USB_LINK=1;
        else
          echo "Waiting for USB link ($USB_DEVICE)..."
          USB_LINK=0;
        fi
        sleep 1;
      done;
    else
      echo "Missing OpenMoko device ..."
    fi
    sleep 1;
  done;
}

# Function continue_or_skip() - Skips or continues next step
# ====================================================================================================
continue_or_skip()
{
  CONTINUE=;
  SKIP=;
  while [ -z "$CONTINUE" ]; do
    read -r -p "Type anything but \"c\" to continue or \"s\" to skip. [c/s]: " CONTINUE; 
  done;
  if [ $CONTINUE = s ]
  then
    SKIP=y
  else
    SKIP=;
  fi
}

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
  file $USB_FLASHER/flash-fx2lp.sh
else
  echo "Error: USB flash script is missing!"  
  exit 1
fi
echo "All files are present!\n"
echo "Compiling the device test ..."
make device-test
echo "\nPlease turn off the complete board, attach the WRPX1 and the addon board and do a power cycle.";


# Flash USB device
# ====================================================================================================
echo "\nStep 2: Flashing USB device";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FLASH_USB = "yes" ]
then
  echo "Please connect the USB connector to the base board.";
  continue_or_skip
  if [ -z "$SKIP" ]; then
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
fi

# Flash FPGA
# ====================================================================================================
echo "\nStep 3: Flashing FPGA";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FLASH_FPGA = "yes" ]
then
  echo "Please connect the JTAG connector to the BASE BOARD and reset it (power cycle).";
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
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
fi

# Format 1-wire
# ====================================================================================================
echo "\nStep 4: Format the 1-wire EEPROM";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FORMAT_OW = "yes" ]
then
  echo "Please do a power cycle";
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
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
fi

# JTAG addon test
# ====================================================================================================
echo "\nStep 5: Check JTAG connection from addon board";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $FLASH_USB = "yes" ]
then
  echo "Please connect the JTAG connector to the ADDON BOARD and reset it (power cycle).";
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
    echo "Programming the Vetar2a"
    $QUARTUS_BIN/quartus_pgm -c 1 -m jtag -o "p;$VETAR2A_SOF_FILE" 
  else
    echo "Skipping this step ..."
  fi
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
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
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
fi

if [ $IO_TEST_STEP2 = "yes" ]
then
  echo "Please connect the following I/Os:";
  echo "- IO1  <=> IN1 (lemo)";
  echo "- IO2  <=> IN2 (lemo)";
  echo "- IO3  <=> IN (lemo - near SFP cag)";
  echo "- HDMI <=> HDMI";
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
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
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
    eb-console $USB_DEVICE
    echo "MAC address set!"
  else
    echo "Skipping this step ..."
  fi
fi

# Set MAC address
# ====================================================================================================
echo "\nStep 8: Check WR LINK";
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
if [ $CHECK_GUI = "yes" ]
then
  echo "Setting MAC address now to 02:ff:00:02:00:XX";
  echo "- Please make sure that the USB cable is connected to the base board"
  echo "- Do a power cycle"
  echo "- Use the command \"gui\""
  echo "- After setting the MAC address press ctrl+c" 
  continue_or_skip
  if [ -z "$SKIP" ]; then
    check_usb_connection
    eb-console $USB_DEVICE
    echo "WR LINK IS UP"
  else
    echo "Skipping this step ..."
  fi
fi

# Finish test
# ====================================================================================================
echo "\nCommissioning script finished successfully!\n"
exit 0;
