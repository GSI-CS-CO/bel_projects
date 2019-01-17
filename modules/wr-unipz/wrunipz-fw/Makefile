TARGET		= wrunipz
#PLATFPATH       = ../../../syn/gsi_exploder5/exploder5_csco_tr
PLATFPATH       = ../../../syn/gsi_scu/control3
PLATFMAKEFILE   = $(PLATFPATH)/Makefile

PLATFORM	:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 TARGET    | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
DEVICE		:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 DEVICE    | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
FLASH		:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 FLASH     | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
SPI_LANES	:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 SPI_LANES | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
RAM_SIZE        := $(shell cat 	$(PLATFMAKEFILE) | grep -m1 RAM_SIZE  | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')

SHARED_SIZE    	?= 8K
USRCPUCLK      	?= 125000
VERSION  	= 00.00.07

.DEFAULT_GOAL	:= fwbin

PATHFW 	        = .
DEBUGLVL    	= 1
EXTRA_FLAGS	?=
CFLAGS 	    	= -I../include -I$(PATHFW) -DDEBUGLEVEL=$(DEBUGLVL) $(EXTRA_FLAGS)

$(info    >>>>)
$(info    building is done by importing the following data from $(PLATFMAKEFILE):)
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


