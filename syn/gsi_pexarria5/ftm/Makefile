TARGET			= ftm
DEVICE			= 5AGXMA3D4
FLASH			= EPCQ256
SPI_LANES		= ASx4
RAM_SIZE		= 131072
SHARED_SIZE     = 98304
USRCPUCLK       = 125000
VERSION         = 8.0.4
RELEASE         = Fallout
DEBUGLVL  		= 0

EBMPATH   = ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  = ../../../modules/prioq2
PATHFTM   = ../../../modules/ftm
PATHFW    = $(PATHFTM)/ftmfw
PATHTOOL  = $(PATHFTM)/ftmx86
PREFIX    ?= /usr/local
CFLAGS    = -I. -I$(PATHFTM)/include -I$(WRPC_IMPORT) -I$(PATHFW) -I$(PRIOPATH) -I$(EBMPATH) -DDEBUGLEVEL=$(DEBUGLVL) -DDIAGNOSTICS -DDM_VERSION="\"$(VERSION)\"" -DDM_RELEASE="\"$(RELEASE)\""

include ../../build.mk

all: $(TARGET).elf

tools:
	$(MAKE) -C $(PATHTOOL) tools
	cp $(PATHFTM)/bin/dm-sched $(PATHFTM)/bin/dm-cmd $(PATHFTM)/lib/libcarpedm.so .

install:
	$(MAKE) -C $(PATHTOOL) install



$(TARGET).elf: $(PATHFW)/main.c $(PATHFW)/dm.c $(INCPATH)/ebm.c \
               $(INCPATH)/mini_sdb.c $(INCPATH)/sdb_add.c $(INCPATH)/irq.c \
               $(WRPC_IMPORT)/printf.c $(WRPC_IMPORT)/uart.c \
               $(WRPC_IMPORT)/vsprintf-full.c $(WRPC_IMPORT)/div64.c

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin dm-sched dm-cmd libcarpedm.so
	$(MAKE) -C $(PATHTOOL) clean
