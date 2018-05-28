TARGET		= ftm
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 131072 
SHARED_SIZE     = 98304 
USRCPUCLK       = 125000
VERSION         = 3.1.0
DEBUGLVL  	= 0 

EBMPATH   = ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  = ../../../modules/prioq2
PATHFTM   = ../../../modules/ftm
PATHFW    = $(PATHFTM)/ftmfw
PATHTOOL  = $(PATHFTM)/ftmx86
PREFIX    ?= /usr/local
CFLAGS    = -I. -I$(PATHFTM)/include -I$(PATHFW) -I$(PRIOPATH) -I$(EBMPATH) -DDEBUGLEVEL=$(DEBUGLVL) -DDIAGNOSTICS -DDM_VERSION="\"$(VERSION)\""

include ../../build.mk

all: $(TARGET).elf tools

tools: 
	$(MAKE) -C $(PATHTOOL) tools
	cp $(PATHFTM)/bin/dm-sched $(PATHFTM)/bin/dm-cmd $(PATHFTM)/lib/libcarpedm.so .

install: 
	$(MAKE) -C $(PATHTOOL) install

 

$(TARGET).elf: $(PATHFW)/main.c $(PATHFW)/dm.c $(INCPATH)/ebm.c 

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin dm-sched dm-cmd libcarpedm.so
	$(MAKE) -C $(PATHTOOL) clean


        


