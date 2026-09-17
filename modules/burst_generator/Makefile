# Build a firmware binary for the burst generator
SHELL           := /bin/bash
TARGET		?= burstgen
PLATFPATH       = ../../syn/gsi_scu/control3
PATH            := $(PWD)/../../toolchain/bin:$(PATH)

# find a target form-factor with the least RAM size (SCU3, Pexarria5, Exploder5)
DEF_RAMSIZE     := $(shell grep -m1 RAM_SIZE "$(PLATFPATH)/Makefile" | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
MIN_RAM_TARGET  := $(shell declare -a targets=( "../../syn/gsi_scu/control3"	"../../syn/gsi_pexarria5/control" "../../syn/gsi_exploder5/exploder5_csco_tr" ) ; \
		path=$(PLATFPATH) ; \
		declare -i min_ramsize=$(DEF_RAMSIZE) ; \
		for idx in "$${!targets[@]}" ; do \
				declare -i target_ramsize=$$(grep -m1 RAM_SIZE "$${targets[$${idx}]}/Makefile" | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g') ; \
				if [ $${target_ramsize} -lt $${min_ramsize} ] ; \
				then path=$${targets[$${idx}]} ; min_ramsize=$${target_ramsize} ; fi ; \
		done ; \
		echo $${path} )

PLATFMAKEFILE   = $(MIN_RAM_TARGET)/Makefile
PLATFORM	:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 TARGET    | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
DEVICE		:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 DEVICE    | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
FLASH		:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 FLASH     | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
SPI_LANES	:= $(shell cat 	$(PLATFMAKEFILE) | grep -m1 SPI_LANES | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
RAM_SIZE        := $(shell cat 	$(PLATFMAKEFILE) | grep -m1 RAM_SIZE  | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')

SHARED_SIZE    	?= 16K
USRCPUCLK      	?= 125000
VERSION  	= 00.01.01

.DEFAULT_GOAL := fwbin

PATHFW 	        = .
PATHSCU       = ../../top/gsi_scu
DEBUGLVL    	= 1
EXTRA_FLAGS	?=
CFLAGS 	        =  -I$(PATHFW) -I$(PATHSCU) \
		   -I$(PATHFW)/../common-libs/include \
		   -DDEBUGLEVEL=$(DEBUGLVL) $(EXTRA_FLAGS) \
		   -DBG_FW_VERSION=0x$(subst .,,$(VERSION))

SRC_FILES       = $(PATHFW)/$(TARGET).c \
		  $(PATHFW)/../common-libs/fw/common-fwlib.c \
		  $(PATHSCU)/cb.c # definitions of MSI message buffer functions
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

$(TARGET).elf: $(SRC_FILES)

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


