TARGET		?= ftm
DEBUGLVL  	= 0 
FLASH		?= EPCQ256
RAM_SIZE	?= 49152 
SHARED_SIZE 	?= 16384
USRCPUCLK   	?= 125000
VERSION     	?= 1.1.0


include ../../../syn/build.mk

EBMPATH   ?= $(TOP)/ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  ?= $(TOP)/modules/prioq2
FTMPATH   ?= $(TOP)/modules/ftm
FWPATH    ?= $(FTMPATH)/ftmfw
PREFIX	  ?= /usr/local
CFLAGS	  += -I$(FWPATH) -I$(PRIOPATH) -I$(FWPATH)/../include -DDEBUGLEVEL=$(DEBUGLVL) -DDIAGNOSTICS -DDM_VERSION="\"$(VERSION)\""

all: $(TARGET).bin $(TARGET).elf


$(TARGET).elf: main.c $(INCPATH)/ebm.c dm.c 


clean::
	rm -f $(FWPATH)/*.o $(FWPATH)/*.a $(FWPATH)/*.elf $(FWPATH)/*.bin

