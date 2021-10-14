bel_projects
============
GSI Timing Gateware and Tools

# Checkout
Just clone our project.
```
git clone https://github.com/GSI-CS-CO/bel_projects.git
```

# First Steps
Make will take care of all submodules and additional toolchains.
```
make
```
Important: Please don't mess around using the "git submodule --fancy option" command!

# Kernel Drivers
This will build VME and PCI(e) drivers.
```
make driver
(optional) make driver-install
```

# Etherbone
Builds basic Etherbone tools and library.
```
make etherbone
(optional) make etherbone-install
```

# Tools (Monitoring and EB-Tools)
Additional tools like eb-console and eb-flash.
```
make tools
(optional) make tools-install
```

# Saftlib
Builds basic Saftlib tools and library.
```
make saftlib
(optional) make saftlib-install
```
For detailed information check ip_cores/saftlib/CompileAndConfigureSaftlib.md.

# Build Gateware(s)
Currently we support a few different form factors.
```
make scu2
make scu3
make vetar2a
make vetar2a-ee-butis
make pexarria5
make exploder5
make pmc
make microtca
make pexp
```

## FAQ
### Which Version of Quartus Do I Need?
We recommend to use Quartus 18.1.0 (Build 625 09/12/2018 SJ)

### Which Packages Are Required?
You need to have installed the following packages before you can configure and build Etherbone and Saftlib:
* docbook-utils
* libglib2.0-dev
* autotools-dev
* autoconf
* libtool (glibtoolize)
* build-essential
* automake
* libreadline-dev

## Common Errors and Warnings
### Error: quartus: error while loading shared libraries: libpng12-0.0: ... [Ubuntu/Mint/...]

#### Ubuntu
Get the package from here: https://packages.ubuntu.com/xenial/amd64/libpng12-0/download

#### Mint
<pre>
sudo add-apt-repository ppa:linuxuprising/libpng12
sudo apt update 
sudo apt install libpng12-0
</pre>

### Error: error while loading shared libraries: libmpfr.so.4: cannot open shared object file: No such file or directory [Ubuntu/Mint/...]
Create a new symlink: sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4

### Error: Executing qmegawiz: child process exited abnormally + Time value XXX,YYYMbps and time unit are illegal
Change your LC_NUMERIC setting: export LC_NUMERIC="en_US.UTF-8"

### Error: hdlmake: AttributeError: 'module' object has no attribute '_vendor' or hdlmake not found
In case a simple "make" does not fix this:
```
apt-get install python-setuptools
./install-hdlmake.sh
```

### Error (23035): Tcl error: couldn't execute "qsys-generate": no such file or directory
Adjust your PATH variable like this:
```
export QUARTUS=/opt/quartus/
export QSYS_ROOTDIR=$QUARTUS/sopc_builder/bin
export PATH=$PATH:$QUARTUS_ROOTDIR:$QSYS_ROOTDIR
```

### Error: cd ip_cores/hdlmake/ && python setup.py install --user /bin/sh: 1: python: not found
In case you are running Ubuntu:
```
sudo apt-get install python-is-python3
```

### Error: quartus: USB-Blaster can't find FPGA [Ubuntu/Mint/...]
Create a new symlink: sudo ln -sf /lib/x86_64-linux-gnu/libudev.so.1 /lib/x86_64-linux-gnu/libudev.so.0

### Error: /bin/sh: 1: hdlmake: not found (Python 2.7)
You should run "make" to install hdlmake locally. In case you're still using Python 2.7 you have to adjust your PATH variable: 
```
export PATH=$PATH:$HOME/.local/bin
```

## JTAG and Programming
### Altera/Intel USB Blaster

See bel_projects/doc/usbblaster/readme.md

### Xilinx Platform Cable II

See bel_projects/doc/platform_cable/readme.md
