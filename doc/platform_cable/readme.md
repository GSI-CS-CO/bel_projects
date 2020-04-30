# Using Xilinx Platform Cable II

## Rules

1. Copy dot rules files to /etc/udev/rules.d/

## Load Firmware

1. $ lsusb | grep Xilinx # Get bus and device ID => Example output: Bus 001 Device 012: ID 03fd:0013 Xilinx, Inc.
2. $ sudo /sbin/fxload -v -t fx2 -I <<YOUR_XILINX_PATH>>/linux_drivers/pcusb/xusb_xp2.hex -D /dev/bus/usb/001/012
