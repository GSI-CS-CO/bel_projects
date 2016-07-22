TARGET		= ftm
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 49152 
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 0.2.4
VERSION_TOOL    = 0.1.4

EBMPATH   = ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  = ../../../modules/prioq2
PATHFTM   = ../../../modules/ftm
PATHFW    = $(PATHFTM)/ftmfw
PATHTOOL  = $(PATHFTM)/ftmx86
PREFIX    ?= /usr/local
CFLAGS    = -I$(PATHFW) -I$(PRIOPATH) -I$(EBMPATH)

include ../../build.mk
include $(PATHTOOL)/Makefile

all: $(TARGET).elf tool

$(TARGET).elf: $(PATHFW)/main.c $(PATHFW)/ftm.c $(INCPATH)/ebm.c 

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


        


