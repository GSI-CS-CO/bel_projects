TARGET		= wrunipz
TRPATH          = ../../../syn/gsi_exploder5/exploder5_csco_tr
TRMAKEFILE      = $(TRPATH)/Makefile

PLATFORM        := $(shell echo $(TRPATH)    | rev | cut -d'/' -f1 | rev )
DEVICE		:= $(shell cat $(TRMAKEFILE) | grep DEVICE    | cut -d' ' -f2 )
FLASH		:= $(shell cat $(TRMAKEFILE) | grep FLASH     | cut -d' ' -f2 )
SPI_LANES	:= $(shell cat $(TRMAKEFILE) | grep SPI_LANES | cut -d' ' -f2 )
RAM_SIZE        := $(shell cat $(TRMAKEFILE) | grep RAM_SIZE  | cut -d' ' -f2 )

SHARED_SIZE     ?= 8K
USRCPUCLK       ?= 125000
VERSION         = 00.00.02

.DEFAULT_GOAL  := fwbin

PATHFW 	        = .
DEBUGLVL    	= 1
CFLAGS 	    	= -I$(PATHFW) -DDEBUGLEVEL=$(DEBUGLVL)

$(info    >>>>)
$(info    building is done by importing following data from $(TRPATH):)
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


