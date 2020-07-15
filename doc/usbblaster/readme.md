# Installing the USB Blaster

## Rules

1. Create the file /etc/udev/rules.d/51-altera-usbblaster.rules
2. Use the local file (51-altera-usbblaster.rules) as example or copy it

## Install missing libraries

1. $ sudo apt-get install libudev1:i386
2. $ sudo ln -sf /lib/x86_64-linux-gnu/libudev.so.1 /lib/x86_64-linux-gnu/libudev.so.0
