About
=====
ftdi-mcp is some c code to work with a comparator box connected to the
host system  via USB. Inside,  there is  usb-serial chip from  ftdi to
which a DAC is connected via I2C. The  DAC is used to set the level of
the comparator. See documentation in folder 'doc'.


Requirements
============
Tested on  linux. Althouth  a kernel driver  for ftdi  exist, ftdi-mcp
uses other libraries from ftdi instead.

- libftd2xx.so.1.4.27
-- low  level library
-- requires  downloading 'libftd2xx-x86_64-1.4.27.tgz' from the internet
- libmpsse.so.1.0.3
-- higher level library required for I2C functionality
--  requires downloading 'LibMPSSE_1.0.3.zip' from the internet

Prior use, the EEPROM of the device must be initialized/formatted. This
can be done either with the program

- FT_PROG (windows) or
- the linux variant
-- use tool contained in 'ft232r_prog-1.25.tar.gz' (download from the internet)
-- format EEPROM via command line
   'sudo ./ftx_prog --old-pid 0x6014 --dump --ignore-crc-error'






