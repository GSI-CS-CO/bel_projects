# Two examples are provided
# - miniExample: 'hello world' for lm32
# - example: demo program including
# -- shared memory for exchange of data via Wishbone
# -- access to other Wishbone devices
# -- receive commands via shared memory
# -- get actions from the ECA by polling
#
# Example usage:
#   make TARGET=example
#   ... will compile the more complex example program

ifndef COLORED
all:
	((make -C $(PWD) COLORED=1) 2>&1 | sed -e 's/.*\bWARN.*/\x1b[7m&\x1b[0m/i' -e 's/.*\bERR.*/\x1b[93;41m&\x1b[0m/i')
endif



TARGET		?= scu_bus_test
.DEFAULT_GOAL := fwbin
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 49152 
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 0.0.1

PATHFW 	= .
PATHPKG	= $(shell python2.7 ../../ip_cores/hdl-make/hdlmake list-mods | grep -G '^[^\#]' | grep top | grep -o '^\S*')
PREFIX 	?= /usr/local
CFLAGS 	= -I$(PATHFW) -I$(PATHFW)/../../../top/gsi_scu/


include ../../../syn/build.mk



fwbin: $(TARGET).bin

$(TARGET).elf: $(PATHFW)/$(TARGET).c $(PATHFW)/../../../top/gsi_scu/scu_bus.c \
               $(PATHFW)/../../../top/gsi_scu/dow_crc.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


