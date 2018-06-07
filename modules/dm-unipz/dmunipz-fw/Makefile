TARGET		= dmunipz
.DEFAULT_GOAL := fwbin
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 65536 
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 00.01.10

PATHFW 	 = .
PATHPKG	 = $(shell python2.7 ../../../ip_cores/hdl-make/hdlmake list-mods | grep -G '^[^\#]' | grep top | grep -o '^\S*')
PREFIX 	 ?= /usr/local
CFLAGS 	 = -I$(PATHFW) -DDEBUGLEVEL=$(DEBUGLVL)
DEBUGLVL = 1

include ../../../syn/build.mk

fwbin: $(TARGET).bin 

$(TARGET).elf: $(PATHFW)/dm-unipz.c $(INCPATH)/ebm.c $(PATHFW)/../../../top/gsi_scu/scu_mil.c

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


