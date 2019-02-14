# The following examples are provided
# - miniExample: 'hello world' for lm32
# - example: demo program including
# -- shared memory for exchange of data via Wishbone
# -- access to other Wishbone devices
# -- receive commands via shared memory
# -- get actions from the ECA by polling
# - milExample.c: communication with MIL device bus
# - milSnooper.c: snoops MIL event bus
# - 
#
# Example usage:
#   make TARGET=example
#   ... will compile the more complex example program
TARGET		?= miniExample
PLATFPATH       = ../../syn/gsi_exploder5/exploder5_csco_tr
#PLATFPATH       = ../../syn/gsi_scu/control3
PLATFMAKEFILE   = $(PLATFPATH)/Makefile

PLATFORM	:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 TARGET    | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
DEVICE		:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 DEVICE    | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
FLASH		:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 FLASH     | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
SPI_LANES	:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 SPI_LANES | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
RAM_SIZE        := $(shell cat 	$(PLATFMAKEFILE) | grep -m1 RAM_SIZE  | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')

SHARED_SIZE    	?= 16K
USRCPUCLK      	?= 125000
VERSION  	= 00.00.01

.DEFAULT_GOAL := fwbin

#is this junk we can get rid of?
#PATHPKG	= $(shell python2.7 ../../ip_cores/hdl-make/hdlmake list-mods | grep -G '^[^\#]' | grep top | grep -o '^\S*')
#PREFIX 	?= /usr/local

PATHFW 	        = .
DEBUGLVL    	= 1
EXTRA_FLAGS	?=
CFLAGS 	        =  -I$(PATHFW) -DDEBUGLEVEL=$(DEBUGLVL) $(EXTRA_FLAGS)


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

include ../../syn/build.mk


fwbin: $(TARGET).bin

$(TARGET).elf: $(PATHFW)/$(TARGET).c $(PATHFW)/../../top/gsi_scu/scu_mil.c

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


