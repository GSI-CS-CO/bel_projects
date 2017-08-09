TARGET		= ftm
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 49152 
SHARED_SIZE     = 8192
USRCPUCLK       = 125000
VERSION         = 0.2.4
VERSION_TOOL    = 0.2.0

EBMPATH   = ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  = ../../../modules/prioq2
PATHFTM   = ../../../modules/ftm
PATHFW    = $(PATHFTM)/ftmfw
PATHTOOL  = $(PATHFTM)/ftmx86
PREFIX    ?= /usr/local
CFLAGS    = -I. -I$(PATHFTM)/include -I$(PATHFW) -I$(PRIOPATH) -I$(EBMPATH)

include ../../build.mk

all: $(TARGET).elf tools

tools: 
	$(MAKE) -C $(PATHTOOL) tools
	cp $(PATHFTM)/bin/dm-sched $(PATHFTM)/bin/dm-cmd $(PATHFTM)/lib/libcarpedm.a .

install: 
	$(MAKE) -C $(PATHTOOL) install

 

$(TARGET).elf: $(PATHFW)/main.c $(PATHFW)/dm.c $(INCPATH)/ebm.c 

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin dm-sched dm-cmd libcarpedm.a
	$(MAKE) -C $(PATHTOOL) clean


        


