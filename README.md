# Project bel_projects

GSI Timing Gateware and Tools

# Table of Contents
- [Build Instructions](#build-instructions)
  - [Checkout](#checkout)
  - [First Steps](#first-steps)
  - [Kernel Drivers](#kernel-drivers)
  - [Etherbone](#etherbone)
  - [Tools (Monitoring and EB-Tools)](#tools-monitoring-and-eb-tools)
  - [Saftlib](#saftlib)
  - [Build Gateware(s)](#build-gatewares)
  - [Additional Targets](#additional-targets)
    - [Check Timing Constraints](#check-timing-constraints)
    - [Sort QSF Files](#sort-qsf-files)
    - [LM32 Cluster Testbench](#lm32-cluster-testbench)
- [FAQ and Common Problems](#faq-and-common-problems)
  - [Synthesis](#synthesis)
    - [Quartus Setup](#quartus-setup)
    - [Quartus Version](#quartus-version)
    - [Library libpng12](#library-libpng12)
      - [Ubuntu](#ubuntu)
      - [Mint](#mint)
      - [Backup Plan](#backup-plan)
    - [Tool qmegawiz](#tool-qmegawiz)
    - [Tool qsys-generate](#tool-qsys-generate)
    - [Permission denied](#permission-denied)
  - [Build Flow](#build-flow)
    - [Required Packages](#required-packages)
    - [Library libmpfr](#library-libmpfr)
    - [Tool hdlmake](#tool-hdlmake)
      - [Tool hdlmake not Found (Python 2.7)](#tool-hdlmake-not-found-python-27)
    - [Python not found](#python-not-found)
    - [No module named pkg_resources](#no-module-named-pkg_resources)
    - [Setuptools not found](#setuptools-not-found)
    - [Python Six](#python-six)
    - [Compiling Saftlib](#compiling-saftlib)
    - [CC not found](#cc-not-found)
    - [Rocky-9](#rocky-9)
    - [Yocto](#yocto)
    - [Package Requirements Etherbone](#package-requirements-etherbone)
  - [Git](#git)
    - [CAfile](#cafile)
  - [JTAG and Programming](#jtag-and-programming)
    - [JTAG Overview](#jtag-overview)
    - [USB-Blaster Issues](#usb-blaster-issues)
    - [Altera/Intel USB Blaster](#alteraintel-usb-blaster)
    - [Xilinx Platform Cable II](#xilinx-platform-cable-ii)
    - [Arrow USB Programmer](#arrow-usb-programmer)
    - [Altera/Intel Ethernet Blaster](#alteraintel-ethernet-blaster)
  - [Timing Receiver](#timing-receiver)
    - [Commissioning](#commissioning)
    - [Flashing](#flashing)
      - [Arria2 Devices](#arria2-devices)
      - [ArriaV Devices](#arriav-devices)
      - [Arria10 Devices](#arria10-devices)

# Build Instructions

## Checkout

Just clone our project.

```shell
git clone https://github.com/GSI-CS-CO/bel_projects.git
```

## First Steps

Make will take care of all submodules and additional toolchains.

```shell
make
```

Important: Please don't mess around using the "git submodule --fancy option" command!

## Kernel Drivers

This will build VME and PCI(e) drivers.

```shell
make driver
make driver-install # optional
make driver/driver-install WISHBONE_SERIAL=y # optional - build wishbone-serial.ko
```

## Etherbone

Builds basic Etherbone tools and library.

```shell
make etherbone
make etherbone-install # optional
```

## Tools (Monitoring and EB-Tools)

Additional tools like eb-console and eb-flash.

```shell
make tools
make tools-install # optional
```

## Saftlib

Builds basic Saftlib tools and library.

```shell
make saftlib
make saftlib-install # optional
```

For detailed information check ip_cores/saftlib/CompileAndConfigureSaftlib.md.

## Build Gateware(s)

Currently we support a few different form factors.

```shell
make scu2               # Arria II
make scu3               # Arria II
make vetar2a            # Arria II
make vetar2a-ee-butis   # Arria II
make ftm                # Arria V
make pexarria5          # Arria V
make exploder5          # Arria V
make pmc                # Arria V
make microtca           # Arria V
make pexp               # Arria V
make scu4slim           # Arria 10
make pexarria10         # Arria 10
make ftm10              # Arria 10
make scu5               # Arria 10
make ftm5               # Arria 10
make ftm5dp             # Arria 10
```

## Additional Targets

### Check Timing Constraints

```
make $device-check
make exploder5-check # example
```

### Sort QSF Files

```
make $device-sort
make exploder5-sort # example
```

### LM32 Cluster Testbench

```
make lm32-cluster-testbench-run
```

[Click here for additional information.](testbench/lm32_cluster/test/REAME.md)

# FAQ and Common Problems

## Synthesis

### Quartus Setup

```shell
export QSYS_ROOTDIR=$CFG_QUARTUS_VERSION/quartus/sopc_builder/bin
export QUARTUS_ROOTDIR=$CFG_QUARTUS_VERSION/quartus
export QUARTUS=$QUARTUS_ROOTDIR
export QUARTUS_64BIT=1
export PATH=$PATH:$QUARTUS
export PATH=$PATH:$QSYS_ROOTDIR
```

Please adjust your `$CFG_QUARTUS_VERSION` variable.

### Quartus Version

Question: Which Version of Quartus Do I Need?

Answer:

- Arria II/Arria V: Quartus 18.1.0 (Build 625 09/12/2018 SJ) Standard Edition
- Arria 10: Quartus 23.1.1 (Build 993 05/14/2024 SC) Standard Edition

### Library libpng12

Error: Quartus error while loading shared libraries: libpng12-0.0: ... [Ubuntu/Mint/...]

Solution: Install the missing package

#### Ubuntu

Get the package from here: https://packages.ubuntu.com/xenial/amd64/libpng12-0/download

#### Mint

```shell
sudo add-apt-repository ppa:linuxuprising/libpng12
sudo apt update
sudo apt install libpng12-0
```

If this PPA can't be added, you need to compile the library:

```shell
cd res/ubuntu-22-and-later
tar xf libpng_1.2.54.orig.tar
cd libpng-1.2.54
./autogen
./configure
make
sudo make install
```

#### Backup Plan

You can use a copy from here:

- Ubuntu: res/ubuntu
- Rocky-9: res/rocky-9

### Tool qmegawiz

Error: Executing qmegawiz: child process exited abnormally + Time value XXX,YYYMbps and time unit are illegal

Solution: Change your LC_NUMERIC setting:

```shell
export LC_NUMERIC="en_US.UTF-8"
```

### Tool qsys-generate

Error: (23035) Tcl error: couldn't execute "qsys-generate": no such file or directory

Solution: Adjust your PATH variable like this:

```shell
export QUARTUS=/opt/quartus/
export QSYS_ROOTDIR=$QUARTUS/sopc_builder/bin
export PATH=$PATH:$QUARTUS_ROOTDIR:$QSYS_ROOTDIR
```

Error: (293007): Current module quartus_sh ended unexpectedly. Verify that you have sufficient memory available to compile your design.

Solution: Use the right Quartus version for your project.

### Permission denied

Error: /bin/sh: 1: cannot create /ramsize_pkg.vhd: Permission denied

Solution: Check all your (changed) Manifest.py files.

## Build Flow

### Required Packages

Question: Which packages are required?

Answer: You need to have installed the following packages before you can configure and build Etherbone and Saftlib:

- docbook-utils
- libglib2.0-dev
- autotools-dev
- autoconf
- libtool (glibtoolize)
- build-essential
- automake
- libreadline-dev
- libsigc++ (saftlib) ‡
- libboost-dev (saftlib)
- pkgconfig (saftlib) †
- xsltproc (saftlib)
- libz-dev (saftlib)

† Ubuntu 22.04 and later: pkg-config

‡ Ubuntu 22.04 and later: libsigc++-2.0-dev

For `apt` that means
```shell
apt install docbook-utils libglib2.0-dev autotools-dev autoconf libtool build-essential automake libreadline-dev libsigc++-2.0-dev libboost-dev pkg-config xsltproc libz-dev python-is-python3
```

For `pacman` that would be
```shell
pacman -S docbook-utils autoconf automake libtool readline libsigc++ pkgconf libxslt glibmm boost
```

### Library libmpfr

Error: error while loading shared libraries: libmpfr.so.4: cannot open shared object file: No such file or directory [Ubuntu/Mint/...]

Error: lm32-* permission denied /dev/stdout

Solution: Create a new symlink:

```shell
sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4
```

### Tool hdlmake

Error: hdlmake AttributeError: module object has no attribute vendor or hdlmake not found

Solution: In case a simple "make" does not fix this:

```shell
make hdlmake_install
```

#### Tool hdlmake not found (for both Python 2.7 & Python 3.x)

Error: /bin/sh: 1: hdlmake: not found

Solution: You should run "make" to install hdlmake locally and adjust your PATH variable:

```shell
export PATH=$PATH:$HOME/.local/bin
```

### Python not found
Error: cd ip_cores/hdlmake/ && python setup.py install --user /bin/sh: 1: python: not found

Solution: In case you are running Ubuntu:

```shell
sudo apt-get install python-is-python3
```

Optional (python-is-python3 not found):

```shell
sudo ln -s /usr/bin/python3 /etc/python
sudo apt-get install python-setuptools
sudo apt-get install python3-setuptools
```

In case you have no sudo rights:

```shell
ln -s /usr/bin/python3 python
export PATH=$PATH:$(pwd)
```

We recommend to use at least Python3.7.

### No module named pkg_resources

Error: ImportError: No module named pkg_resources

Solution:

```shell
sudo apt-get install python-pkg-resources
sudo apt-get install --reinstall python-pkg-resources # if already installed
```

### Setuptools not found

Error: ModuleNotFoundError: No module named 'setuptools'

Solution: Just install the right setuptools:

```shell
sudo apt-get install python3-setuptools # Python 3.X
sudo apt-get install python-setuptools # Python 2.X
```

### Python Six

Error: ModuleNotFoundError: No module named 'six'

Solution:

```shell
sudo apt-get install python-six
sudo apt-get install python3-six
```

### Compiling Saftlib

Error: Compilation: "Error message: ./configure: line 16708: syntax error near unexpected token 0.23' ./configure: line 16708: PKG_PROG_PKG_CONFIG(0.23)'"

Solution:

```shell
sudo apt-get install pkg-config
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```

Error: Compillation: "saftbus/process.cpp:14:10: fatal error: linux/ioprio.h: No such file or directory"

Solution:

1. In Makefile.am

  - line 405: delete 'saft-roundtrip-latency saft-standalone-roundtrip-latency'
  - delete lines 486-490

```make
saft_roundtrip_latency_LDADD = $(EB_LIBS)  $(SIGCPP_LIBS) libsaftbus.la libsaft-proxy.la -ldl #-lltdl
saft_roundtrip_latency_SOURCES = src/saft-roundtrip-latency.cpp

saft_standalone_roundtrip_latency_LDADD = $(EB_LIBS)  $(SIGCPP_LIBS) libsaftbus.la libsaft-service.la  -ldl #-lltdl
saft_standalone_roundtrip_latency_SOURCES = src/saft-standalone-roundtrip-latency.cpp
```

2. In saftbus/process.cpp, replace the entire code by

```cpp
#include "process.hpp"

#include <iostream>
#include <sstream>
#include <cstring>

bool set_realtime_scheduling(std::string argvi, char *prio) {
  return true;
}

bool set_cpu_affinity(std::string argvi, char *affinity) {
  return true;
}

bool set_ioprio(char *ioprio) {
  return true;
}
```

### CC not found

Error: make[1]: cc: No such file or directory

Solution:

```shell
which cc # cc: Command not found.
update-alternatives --list cc
which cc # /usr/bin/cc
```

<<<<<<< HEAD
### Rocky-9

[Click here for additional information.](res/rocky-9)

### Yocto

#### Etherbone & Saftlib

```shell
unset LD_LIBRARY_PATH
source /common/usr/embedded/yocto/sdk/environment-setup-core2-64-ffos-linux
make etherbone YOCTO_BUILD=yes
make saftlib YOCTO_BUILD=yes
```

Check the Rocky-9 subsection, if you get lsb_release related errors.

#### Etherbone Tools

See [tools/yocto-build.sh](tools/yocto-build.sh)

### Package Requirements Etherbone

Error: configure: error: Package requirements (etherbone >= x.y.z) were not met:

Solution:

```
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```
=======
### Rocky Linux

For Rocky Linux v9.x support please check this file: [README.md](#res/rocky-9/README.md)
>>>>>>> a97b45185 (README.md: add rocky-9 note)

## Git

### CAfile

Error: Cloning into 'dir'... - fatal: unable to access 'https://ohwr.org/project/generic_project.git/': server certificate verification failed. CAfile: none CRLfile: none

Solution: Systems with outdated trust databases (root CA certificate Let's Encrypt) will be unable to validate the certificate of the site. Update ca-certificates to fix this:

```shell
sudo apt update
sudo apt upgrade ca-certificates
```

## JTAG and Programming

### JTAG Overview

| Timing Receiver         | JTAG Adapter(s)                | JTAG Adapter Configuration   |
| ----------------------- | ------------------------------ | ---------------------------- |
| SCU2 & SCU3             | Promo 11 (Micro-USB)           | - |
| Vetar2a                 | Promo 11 (Micro-USB) <br> Promo 5 | Promo 5: SEL1 0x1 - SEL2 0x4 |
| Pexarria5               | Promo 5 <br> Promo 12          | FPGA Promo 5: SEL1 0x2 - SEL2 0x8 <br> CPLD Promo 5: SEL1 0x1 - SEL2 0x4 <br> FPGA Promo 12: SEL1 0x2 - SEL2 0x8 (?) <br> CPLD Promo 12: SEL1 0x1 - SEL2 0x4 (?) |
| Exploder5               | Promo 11 (Micro-USB) <br> Promo 5 <br> Promo 12 | FPGA Promo 5: SEL1 0x1 - SEL2 0x4 <br> CPLD Promo 5: SEL1 0x2 - SEL2 0x8 <br> FPGA Promo 12: SEL1 0x1 - SEL2 0x4 (?) <br> CPLD Promo 12: SEL1 0x2 - SEL2 0x8 (?) |
| MicroTCA/AMC            | Promo 11 (Micro-USB)           | FPGA/CPLD JTAG chain |
| PMC                     | Promo 11 (Micro-USB)           | FPGA/CPLD JTAG chain |
| PEXP                    | Promo 11 (Micro-USB)           | FPGA/CPLD JTAG chain |
| SCU4 & FTM4             | Promo 11 (Micro-USB)           | - |
| SCU4.1 (SCU4SLIM)       | Promo 11 (Micro-USB)           | USB-Blaster II only :warning: |
| Pexarria10 & FTM10      | Promo 11 (Micro-USB)           | Optional USB to JTAG adapter † |
| SCU5                    | Promo 11 (Micro-USB)           | Optional USB to JTAG adapter † |

† If attached: [FPGA USB-Programmer2 JTAG (Arrow)](https://shop.trenz-electronic.de/de/TEI0004-02-FPGA-USB-Programmer2-JTAG-Arrow-fuer-die-Entwicklung-mit-Intel-FPGAs?c=26)

### USB-Blaster Issues

Error: quartus: USB-Blaster can't find FPGA [Ubuntu/Mint/...]

Solution: Create a new symlink:

```shell
sudo ln -sf /lib/x86_64-linux-gnu/libudev.so.1 /lib/x86_64-linux-gnu/libudev.so.0
```

### Altera/Intel USB Blaster

See [doc/usbblaster/readme.md](doc/usbblaster/readme.md)

### Xilinx Platform Cable II

See [doc/platform_cable/readme.md](doc/platform_cable/readme.md)

### Arrow USB Programmer

See [doc/arrow_usb_programmer/readme.md](doc/arrow_usb_programmer/readme.md)

### Altera/Intel Ethernet Blaster

```shell
Default user: admin
Default password: password
Default server port (programmer GUI): 1309
```

## Timing Receiver

### Commissioning

Configure the SPI flash chip:

```shell
eb-config-nv $device 10 4
```

Format the 1-wire EEPROM:

```shell
cd bel_projects/ip_cores/wrpc-sw/tools
eb-w1-write $device 0 320 < sdb-wrpc.bin
```

Program FPGA from command line:

```shell
quartus_pgm -c 1 -m jtag -o 'p;device.sof'
```

### Flashing

Problem: Flashing might fail sometimes on certain devices and host combinations.

Solution: If you have such a device please use eb-flash (with additional arguments) to flash the timing receiver:

Optional (BEFORE using eb-flash):

```shell
eb-reset $device wddisable # disable watchdog timer
eb-reset $device cpuhalt 0xff # stop all embedded CPUs
```

Optional (AFTER using eb-flash):
```shell
eb-reset $device fpgareset # reset FPGA
```

#### Arria2 Devices

```shell
(problematic devices) eb-flash -s 0x40000 -w 3 $device $gateware.rpd # <VETAR2A/VETAR2A-EE-BUTIS/SCU2/SCU3>
(unproblematic devices) eb-flash $device $gateware.rpd # <VETAR2A/VETAR2A-EE-BUTIS/SCU2/SCU3>
```

#### ArriaV Devices

```shell
(problematic devices) eb-flash -s 0x10000 -w 3 $device $gateware.rpd # <PEXP/PEXARRIA5/PMC/MICROTCA/EXPLODER5>
(unproblematic devices) eb-flash $device $gateware.rpd # <PEXP/PEXARRIA5/PMC/MICROTCA/EXPLODER5>
```

#### Arria10 Devices

```shell
eb-asmi $device -w $gateware.rpd (write)
eb-asmi $device -v $gateware.rpd (verify)
```
