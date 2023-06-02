PLATFMAKEFILE    := $(PLATFPATH)/Makefile

# variables (actually RAM_SIZE) are exported to allow recursive build of ram.ld in top make
export PLATFORM  := $(shell grep -m1 TARGET    $(PLATFMAKEFILE) | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
export DEVICE    := $(shell grep -m1 DEVICE    $(PLATFMAKEFILE) | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
export FLASH     := $(shell grep -m1 FLASH     $(PLATFMAKEFILE) | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
export SPI_LANES := $(shell grep -m1 SPI_LANES $(PLATFMAKEFILE) | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')
export RAM_SIZE  := $(shell grep -m1 RAM_SIZE  $(PLATFMAKEFILE) | cut -d'=' -f2 | sed 's/[^a-zA-Z0-9]//g')

# obtain the number of MPS channels
ifneq ($(MPS_CH),)
  N_MPS_CH       := $(shell $(CC) -dM -E -D$(MPS_CH) fbas_common.h | sed -n 's|.*N_MPS_CHANNELS[[:space:]]*||p')
else
  N_MPS_CH       := $(shell $(CC) -dM -E fbas_common.h | sed -n 's|.*N_MPS_CHANNELS[[:space:]]*||p')
endif

$(info    N_MPS_CH    is $(N_MPS_CH))

# use N_MPS_CH to rename a target to fbas<ch>.<platform>.bin (where, ch='' if N_MPS_CH=1)
ifneq ($(N_MPS_CH),1)
  ifeq ($(TARGET),fbas)
    export N_MPS_CH
  else
    undefine N_MPS_CH
  endif
else
  undefine N_MPS_CH
endif

CFLAGS        = -I../include -I../../common-libs/include -I../../wb_timer -I../../../ip_cores/saftlib/src -I$(PATHFW) \
                -DPLATFORM=$(PLATFORM) -DDEBUGLEVEL=$(DEBUGLVL) $(EXTRA_FLAGS)

ifneq ($(MPS_CH),)
  CFLAGS += -D$(MPS_CH)
endif

SRC_FILES     = $(PATHFW)/$(TARGET).c  \
		$(PATHFW)/fwlib.c $(INCPATH)/ebm.c $(PATHFW)/../../common-libs/fw/common-fwlib.c

ifeq ($(TARGET),fbas)
  SRC_FILES  += $(PATHFW)/tmessage.c $(PATHFW)/ioctl.c $(PATHFW)/measure.c $(PATHFW)/timer.c
endif

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
$(info    CFLAGS      is $(CFLAGS))
$(info    SRC_FILES   is $(SRC_FILES))
$(info    <<<<)

include ../../../syn/build.mk

fwbin: $(TARGET).bin
	@mv $^ $(TARGET)$(N_MPS_CH).$(PLATFORM).bin

$(TARGET).elf: $(SRC_FILES)
