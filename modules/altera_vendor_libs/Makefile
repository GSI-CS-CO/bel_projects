GHDL_VENDOR_PATH ?= /usr/local/lib/ghdl/vendors
GHDL_VENDOR_FLAGS ?= --vhdl2008 --altera --arria
GHDL_VENDOR_COMPILE ?= compile-altera.sh
QUARTUS_DIR ?= $(QUARTUS)/eda/sim_lib/

all:
	$(GHDL_VENDOR_PATH)/$(GHDL_VENDOR_COMPILE) $(GHDL_VENDOR_FLAGS) --source $(QUARTUS_DIR)

clean:
	rm -rf altera || true
	$(GHDL_VENDOR_PATH)/$(GHDL_VENDOR_COMPILE) --clean

help:
	$(GHDL_VENDOR_PATH)/$(GHDL_VENDOR_COMPILE) --help
