TARGET		= ftm
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 49152 
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 0.2.1

EBMPATH   = ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  = ../../../modules/prioq2
PATHFTM   = ../../../modules/ftm
PATHFW    = $(PATHFTM)/ftmfw
PATHTOOL  = $(PATHFTM)/ftmx86
PREFIX    ?= /usr/local
CFLAGS    = -I$(PATHFW) -I$(PRIOPATH) -I$(EBMPATH)

include ../../build.mk

.PHONY: all

all: tool $(TARGET).elf

install: tool
	cp $(PATHTOOL)/ftm-ctl $(PREFIX)/bin/

$(TARGET).elf: $(PATHFW)/main.c $(PATHFW)/ftm.c $(INCPATH)/ebm.c 

tool:
	$(MAKE) -C $(PATHTOOL)

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin
	$(MAKE) -C $(PATHTOOL) clean

        


