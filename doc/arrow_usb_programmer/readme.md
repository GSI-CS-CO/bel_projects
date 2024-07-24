Arrow USB Programmer Hardware Library for Linux, 64 bit
Version 2.2

This is the readme file for the 64-bit Linux programmer hardware library
for BeMicro boards and other Arrow USB Programmer compatible hardware.

Installation
Be sure you have the right permissions for doing as follows.

(1) Copy the file libjtag_hw_arrow.so to the directory linux64
    of thw Quartus installation directory e.g.
    /usr/local/intelFPGA_lite/18.0/quartus/linux64 .

(2) Copy the file 51-arrow-programmer.rules to /etc/udev/rules.d .

The frequency of the JTAG clock TCK can be configured using an additional
configuration file. The programmer hardware library searches the configuration
data using different file names at two different locations.
1. as ".arrow_usb_blaster.conf" in the home directory of the user who has 
   started the JTAG demon. This possibility is intended to set the frequency
   for an individual user or project.
2. as "arrow_usb_blaster.conf" under "/etc" if it was not found in the home
   directory. This is to change the settings globally.
If no configuration file is found the default setting is used which is 20 MHz.

If there is the need to change the JTAG clock frequency, copy the file
"arrow_usb_blaster.conf" and adjust the frequency to your needs.

The programmer shared library uses the FTDI library libftd2xx version 1.4.8
which is staticaly linked.

The programmer shared library for Linux has been developed and tested under
Ubuntu 16.04 LTS. However it should also work with other Linux distributions.

-----
Known issues:

(1) During the enumeration process, the FTDI USB controller performs a reset.
Depending on the hardware used, this might remove the power from the FPGA and
therefore might erase the contents of the FPGA. 
