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
- [FAQ and Common Problems](#faq-and-common-problems)
  - [Synthesis](#synthesis)
    - [Quartus Version](#quartus-version)
    - [Library libpng12](#library-libpng12)
      - [Ubuntu](#ubuntu)
      - [Mint](#mint)
    - [Tool qmegawiz](#tool-qmegawiz)
    - [Tool qsys-generate](#tool-qsys-generate)
  - [Build Flow](#build-flow)
    - [Required Packages](#required-packages)
    - [Library libmpfr](#library-libmpfr)
    - [Tool hdlmake](#tool-hdlmake)
      - [Tool hdlmake not found (Python 2.7)](#tool-hdlmake-not-found-python-27)
    - [Python not found](#python-not-found)
    - [Setuptools not found](#setuptools-not-found)
  - [Git](#git)
    - [CAfile](#cafile)
  - [JTAG and Programming](#jtag-and-programming)
    - [USB-Blaster Issues](#usb-blaster-issues)
    - [Altera/Intel USB Blaster](#alteraintel-usb-blaster)
    - [Xilinx Platform Cable II](#xilinx-platform-cable-ii)
    - [Arrow USB Programmer](#arrow-usb-programmer)
    - [Altera/Intel Ethernet Blaster](#alteraintel-ethernet-blaster)
  - [Timing Receiver](#timing-receiver)
    - [Flashing](#flashing)
      - [Arria2 Devices](#arria2-devices)
      - [ArriaV Devices](#arriav-devices)
      - [Arria10 Devices](#arria10-devices)

# Build Instructions
## Checkout
Just clone our project.
```
git clone https://github.com/GSI-CS-CO/bel_projects.git
```

## First Steps
Make will take care of all submodules and additional toolchains.
```
make
```
Important: Please don't mess around using the "git submodule --fancy option" command!

## Kernel Drivers
This will build VME and PCI(e) drivers.
```
make driver
(optional) make driver-install
(optional - build wishbone-serial.ko) make driver/driver-install WISHBONE_SERIAL=y
```

## Etherbone
Builds basic Etherbone tools and library.
```
make etherbone
(optional) make etherbone-install
```

## Tools (Monitoring and EB-Tools)
Additional tools like eb-console and eb-flash.
```
make tools
(optional) make tools-install
```

## Saftlib
Builds basic Saftlib tools and library.
```
make saftlib
(optional) make saftlib-install
```
For detailed information check ip_cores/saftlib/CompileAndConfigureSaftlib.md.

## Build Gateware(s)
Currently we support a few different form factors.
```
make scu2
make scu3
make scu4
make ftm4
make vetar2a
make vetar2a-ee-butis
make pexarria5
make exploder5
make pmc
make microtca
make pexp
make pexarria10
make ftm10
```

# FAQ and Common Problems

## Synthesis
### Quartus Version
Question: Which Version of Quartus Do I Need?

Answer: We recommend to use Quartus 18.1.0 (Build 625 09/12/2018 SJ)

### Library libpng12
Error: Quartus error while loading shared libraries: libpng12-0.0: ... [Ubuntu/Mint/...]

Solution: Install the missing package

#### Ubuntu
Get the package from here: https://packages.ubuntu.com/xenial/amd64/libpng12-0/download

#### Mint
```
sudo add-apt-repository ppa:linuxuprising/libpng12
sudo apt update
sudo apt install libpng12-0
```

### Tool qmegawiz
Error: Executing qmegawiz: child process exited abnormally + Time value XXX,YYYMbps and time unit are illegal

Solution: Change your LC_NUMERIC setting: 

```
export LC_NUMERIC="en_US.UTF-8"
```

### Tool qsys-generate
Error: (23035) Tcl error: couldn't execute "qsys-generate": no such file or directory

Solution: Adjust your PATH variable like this:

```
export QUARTUS=/opt/quartus/
export QSYS_ROOTDIR=$QUARTUS/sopc_builder/bin
export PATH=$PATH:$QUARTUS_ROOTDIR:$QSYS_ROOTDIR
```

## Build Flow
### Required Packages
Question: Which Packages Are Required?

Answer: You need to have installed the following packages before you can configure and build Etherbone and Saftlib:

- docbook-utils
- libglib2.0-dev
- autotools-dev
- autoconf
- libtool (glibtoolize)
- build-essential
- automake
- libreadline-dev

### Library libmpfr
Error: error while loading shared libraries: libmpfr.so.4: cannot open shared object file: No such file or directory [Ubuntu/Mint/...]

Solution: Create a new symlink:
```
sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4
```

### Tool hdlmake
Error: hdlmake AttributeError: module object has no attribute vendor or hdlmake not found

Solution: In case a simple "make" does not fix this:
```
make hdlmake_install
```

#### Tool hdlmake not found (Python 2.7)
Error: /bin/sh: 1: hdlmake: not found

Solution: You should run "make" to install hdlmake locally. In case you're still using Python 2.7 you have to adjust your PATH variable:

```
export PATH=$PATH:$HOME/.local/bin
```

### Python not found
Error: cd ip_cores/hdlmake/ && python setup.py install --user /bin/sh: 1: python: not found

Solution: In case you are running Ubuntu:

```
sudo apt-get install python-is-python3
```

Optional (python-is-python3 not found):

```
sudo ln -s /usr/bin/python3 /etc/python
sudo apt-get install python-setuptools
```

We recommend to use at least Python3.7.

### Setuptools not found
Error: ModuleNotFoundError: No module named 'setuptools'

Solution: Just install the right setuptools:

```
sudo apt-get install python3-setuptools # Python 3.X
sudo apt-get install python-setuptools # Python 2.X
```

## Git
### CAfile
Error: Cloning into 'dir'... - fatal: unable to access 'https://ohwr.org/project/generic_project.git/': server certificate verification failed. CAfile: none CRLfile: none

Solution: Systems with outdated trust databases (root CA certificate Let's Encrypt) will be unable to validate the certificate of the site. Update ca-certificates to fix this:

```
sudo apt update
sudo apt upgrade ca-certificates
```

## JTAG and Programming

### USB-Blaster Issues
Error: quartus: USB-Blaster can't find FPGA [Ubuntu/Mint/...]

Solution: Create a new symlink:

```
sudo ln -sf /lib/x86_64-linux-gnu/libudev.so.1 /lib/x86_64-linux-gnu/libudev.so.0
```

### Altera/Intel USB Blaster

See bel_projects/doc/usbblaster/readme.md

### Xilinx Platform Cable II

See bel_projects/doc/platform_cable/readme.md

### Arrow USB Programmer

See bel_projects/doc/arrow_usb_programmer/readme.md

### Altera/Intel Ethernet Blaster

<pre>
Default user: admin
Default password: password
Default server port (programmer GUI): 1309
</pre>

## Timing Receiver

### Flashing

Problem: Flashing might fail sometimes on certain devices and host combinations.

Solution: If you have such a device please use eb-flash (with additional arguments) to flash the timing receiver:

Optional (BEFORE using eb-flash):
```
eb-reset $device wddisable # disable watchdog timer
eb-reset $device cpuhalt 0xff # stop all embedded CPUs
```

Optional (AFTER using eb-flash):
```
eb-reset $device fpgareset # reset FPGA
```

#### Arria2 Devices
```
(problematic devices) eb-flash -s 0x40000 -w 3 $device $gateware.rpd # <VETAR2A/VETAR2A-EE-BUTIS/SCU2/SCU3>
(unproblematic devices) eb-flash $device $gateware.rpd # <VETAR2A/VETAR2A-EE-BUTIS/SCU2/SCU3>
```

#### ArriaV Devices
```
(problematic devices) eb-flash -s 0x10000 -w 3 $device $gateware.rpd # <PEXP/PEXARRIA5/PMC/MICROTCA/EXPLODER5>
(unproblematic devices) eb-flash $device $gateware.rpd # <PEXP/PEXARRIA5/PMC/MICROTCA/EXPLODER5>
```

#### Arria10 Devices
```
eb-asmi $device -w $gateware.rpd (write)
eb-asmi $device -v $gateware.rpd (verify)
```
