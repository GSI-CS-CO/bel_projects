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
-- requires downloading 'libftd2xx-x86_64-1.4.27.tgz' from the internet
- libmpsse.so.1.0.3
-- higher level library required for I2C functionality
--  requires downloading 'LibMPSSE_1.0.3.zip' from the internet

Prior use, the EEPROM of the device must be initialized/formatted. This
can be done either with the program FT_PROG (windows) or the linux variant as follows:
- tool contained in 'ft232r_prog-1.25.tar.gz' (download from the internet)
- format EEPROM via command line
  'sudo ./ftx_prog --old-pid 0x6014 --dump --ignore-crc-error'

Install
=======
- follow the instructions in the downloaded libftd2xx-x86_64-1.4.27.tgz and LibMPSSE_1.0.3.zip
-- copy libraries to /usr/local/lib
   (or /opt/usr/lib or ...)
-- create symbolic links
--- libftd2xx.so -> libftd2xx.so.1.4.27
    libmpsse.so  -> libmpsse.so.1.0.3
-- copy header files to /usr/local/include
   (or /opt/usr/include or ...)

Build
=====
- 'cd ~/<your git checkout>/modules/ftdi-mcp/x86/'
- 'make clean'
- 'make'
- this will build the binary 'ftdimcp-ctl'

Usage
=====
Connect the  device to the  host system via  USB.  The device  will be
recognized by the  ftdi_sio driver in the linux  kernel. However, this
driver should not  be used. There are multiple solutions  of not using
the driver. As an example, one could do 'sudo rmmod ftdi_sio' followed
by  'sudo rmmod  usbserial'.  Or  one  could blacklist  the device  by
creating udev rules.  Another option is to 'unbind' the device:

- 'sudo dmesg | grep ftdi'
  [4155047.954863] usbcore: registered new interface driver ftdi_sio
  [4155047.954934] ftdi_sio 2-2:1.0: FTDI USB Serial Device converter detected

- remember the '2-2:1.0', this is the USB connection used by your device

- 'echo -n 2-2:1.0 | sudo tee /sys/bus/usb/drivers/ftdi_sio/unbind'
  (you will need to replace the substring '2-2:1.0' by one of of your device)

- 'sudo dmesg | grep ftdi', just to check
  [4229037.634111] ftdi_sio ttyUSB0: FTDI USB Serial Device converter now disconnected from ttyUSB0
  [4229037.634133] ftdi_sio 2-2:1.0: device disconnected

It should be possible to use the device
- './ftdimcp-ctl -h'      displays help
- './ftdimcp-ctl 0 -i'    displays information like
  description: USB <->  Serial Converter
  serial     : FT7RXPCP
  locId      : 0x104
  ID         : 0x4036014
  type       : 0x8
  flags      : 0x3
- './ftdimcp-ctl 0 -l 20' sets comparator level to 20%


Some Downloads
==============
see here: https://git.gsi.de/aco-tos/downloads/-/tree/main/ftdi








