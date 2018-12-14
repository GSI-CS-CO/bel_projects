TARGET		= wrunipz
PLATFPATH       = ../../../syn/gsi_exploder5/exploder5_csco_tr
PLATFMAKEFILE   = $(PLATFPATH)/Makefile

PLATFORM        := $(shell echo $(PLATFPATH)     | rev | cut -d'/' -f1 | rev )
DEVICE		:= $(shell cat 	$(PLATFMAKEFILE) | grep DEVICE    | cut -d' ' -f2 )
FLASH		:= $(shell cat 	$(PLATFMAKEFILE) | grep FLASH     | cut -d' ' -f2 )
SPI_LANES	:= $(shell cat 	$(PLATFMAKEFILE) | grep SPI_LANES | cut -d' ' -f2 )
RAM_SIZE        := $(shell cat 	$(PLATFMAKEFILE) | grep RAM_SIZE  | cut -d' ' -f2 )

SHARED_SIZE     ?= 8K
USRCPUCLK       ?= 125000
VERSION         = 00.00.02

.DEFAULT_GOAL  := fwbin

PATHFW 	        = .
DEBUGLVL    	= 1
CFLAGS 	    	= -I$(PATHFW) -DDEBUGLEVEL=$(DEBUGLVL)

$(info    >>>>)
$(info    building is done by importing the following data from $(PLATFPATH):)
$(info    PLATFORM    is $(PLATFORM))
$(info    DEVICE      is $(DEVICE))
$(info    FLASH       is $(FLASH))
$(info    SPI_LANES   is $(SPI_LANES))
$(info    RAM_SIZE    is $(RAM_SIZE))
$(info    ----)
$(info    building firmware using)
$(info    SHARED_SIZE is $(SHARED_SIZE))
$(info    USRCPUCLK   is $(USRCPUCLK))
$(info    VERSION     is $(VERSION))
$(info    <<<<)


include ../../../syn/build.mk

fwbin: $(TARGET).bin 

$(TARGET).elf: $(PATHFW)/wr-unipz.c $(INCPATH)/ebm.c $(PATHFW)/../../../top/gsi_scu/scu_mil.c

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


