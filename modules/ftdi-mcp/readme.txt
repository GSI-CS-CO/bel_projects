About
=====
ftdi-mcp is some c code to work with a comparator box connected to the
host system  via USB. Inside,  there is  usb-serial chip from  ftdi to
which a DAC is connected via I2C. The  DAC is used to set the level of
the comparator. See documentation in folder 'doc'.


Requirements
============
FTDI Libraries
--------------
Tested on  linux. Althouth  a kernel driver  for ftdi  exist, ftdi-mcp
uses other libraries from ftdi instead.

- libftd2xx.so.1.4.27
-- low  level library
-- requires downloading 'libftd2xx-x86_64-1.4.27.tgz' from the internet
- libmpsse.so.1.0.3
-- higher level library required for I2C functionality
--  requires downloading 'LibMPSSE_1.0.3.zip' from the internet

EEPROM Config
-------------
On linux, the FTDI chip does not work with the stock EEPROM content

Option A1: Use the FT_PROG tool from FTDI on >>Windows<< and just re-init
           the EEPROM

Option A2: Use the FT_PROG tool from FTDI on >>Windows<< and just init
           the EEPROM using the 'template' FTDI-MCP.xml, available in
           the same folder as this readme
           (FT_PROG tool -> open template...)
Option B1: Use the ftx-prog on >>Linux<< and just re-init the EEPROM by
           'sudo ./ftx_prog --old-pid 0x6014 --dump --ignore-crc-error'
           However, this will make the LEDs unusable.

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

Caveats
=======
ftdi-sio driver
---------------
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

USB stuck
---------
This was observed with kernel 3 on an SCU ramdisk after reboot. The following script solves
the problem by resetting ALL USB 1/2/3 attached ports, see [1]
"
for i in /sys/bus/pci/drivers/[uoex]hci_hcd/*:*; do
  [ -e "$i" ] || continue
  echo "${i##*/}" > "${i%/*}/unbind"
  echo "${i##*/}" > "${i%/*}/bind"
done
"
[1] https://askubuntu.com/questions/645/how-do-you-reset-a-usb-device-from-the-command-line

Usage
=====
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








