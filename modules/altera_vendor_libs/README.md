# Altera Vendor Libraries

Compile vendor libraries for GHDL.

## Building Libraries

Just run `make`. Running the make command may still cause to some issues. Check the altera folder to see if all the libraries you need have been created.

## Error Handling

### GHDL Version

Make sure that you have the latest GHDL version.

```bash
git clone https://github.com/ghdl/ghdl.git
cd ghdl
./configure --prefix=/usr/local
make
sudo make install
```

Source: https://ghdl.github.io/ghdl/development/building/index.html

### Missing compile-altera.sh

Error:

`make: /usr/local/lib/ghdl/vendors/compile-altera.sh: No such file or directory`

Solution:

1. Find the missing script file: `locate compile-alteras.sh`
2. Run make and provide the right GHDL_VENDOR_PATH argument: `make GHDL_VENDOR_PATH=/usr/lib/ghdl/vendors/`

### Unfiltered line

Error:

`ARCHITECTURE RTL OF arriaii_pciehip_iei_detect IS SCRIPT ERROR: Unfiltered line`

Solution:

TL;DR: Just ignore this warning.
