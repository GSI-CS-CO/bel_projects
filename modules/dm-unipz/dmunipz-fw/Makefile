TARGET		= dmunipz
.DEFAULT_GOAL := fwbin
DEVICE		= EP2AGX125
FLASH		= EPCS128
RAM_SIZE	= 98304
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 00.03.02

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


