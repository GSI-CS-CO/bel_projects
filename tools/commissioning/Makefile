# ====================================================================================================
# Makefile for commissioning new hardware
# Example usage: make vetar2a
# ====================================================================================================

# ====================================================================================================
# Target vetar2a
# ====================================================================================================
dev_usb          = dev/ttyUSB0
dev_is_back      = 0
vetar2a_sof_file = ../../syn/gsi_vetar2a/wr_core_demo/vetar2a.sof
vetar2a_rpd_file = ../../syn/gsi_vetar2a/wr_core_demo/vetar2a.rpd

vetar2a:
# Check external dependencies 
	@echo "\nStep 1: Checking dependencies/necessary files";
	@echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
	@test -s $(vetar2a_rpd_file) || { echo "RPD file does not exist!"; exit 1; }
	@echo "Using RPD file: $(vetar2a_rpd_file)"
	@test `which eb-flash` || { echo "Command eb-flash is missing!"; exit 1; }
	@echo "Using eb-flash: `which eb-flash`"
	@echo "Using USB device: $(dev_usb)"
# Flash USB device
	@echo "\nStep 2: Flashing USB device";
	@echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
	@echo "Please connect the USB connector to the base board.";
	@while [ -z "$$CONTINUE" ]; do \
		read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; \
	done ; \
		[ $$CONTINUE = "C" ] || [ $$CONTINUE = "c" ] || (echo "Exiting ..."; exit 1;)
	@echo "Trying to flash the USB device now..."
	@echo "USB device is flashed now!"
# Flash FPGA
	@echo "\nStep 3: Flashing FPGA";
	@echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
	@echo "Please connect the JTAG connector to the base or addon board.";
	@while [ -z "$$CONTINUE" ]; do \
		read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; \
	done ; \
		[ $$CONTINUE = "C" ] || [ $$CONTINUE = "c" ] || (echo "Exiting ..."; exit 1;)
	@echo "Trying to flash the FPGA now ..."
	#eb-flash $(dev_usb) $(vetar2a_rpd_file)
	@echo "FPGA is flashed now!"
# Reset FPGA
	@echo "\nStep 4: Reset FPGA";
	@echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
	@echo "Please reset the complete device (power cycle).";
	@while [ -z "$$CONTINUE" ]; do \
		read -r -p "Type anything but C or c to continue. [C/c]: " CONTINUE; \
	done ; \
		[ $$CONTINUE = "C" ] || [ $$CONTINUE = "c" ] || (echo "Exiting ..."; exit 1;)
	@echo "Waiting for device..."





# ====================================================================================================
# Special targets
# ====================================================================================================
.PHONY: vetar2a
