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
Get the package from here: https://packages.ubuntu.com/xenial/amd64/libpng12-0/download

### Error: error while loading shared libraries: libmpfr.so.4: cannot open shared object file: No such file or directory [Ubuntu/Mint/...]
Create a new symlink: sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4

### Error: Executing qmegawiz: child process exited abnormally + Time value XXX,YYYMbps and time unit are illegal
Change your LC_NUMERIC setting: export LC_NUMERIC="en_US.UTF-8"

