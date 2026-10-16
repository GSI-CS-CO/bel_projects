TARGET		= scu_addac
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PADDAC    = ../../top/gsi_addac
W1        = ../../ip_cores/wrpc-sw
CFLAGS    = -I$(PHK)/include -I$(PADDAC)
USRCPUCLK = 125000

include ../build.mk

# housekeeping uses its own w1-hw.c (SDB-discovered BASE_ONEWIRE)
INCLUDES := $(filter-out $(WRPC_IMPORT)/dev/w1-hw.c,$(INCLUDES))

$(TARGET).sof:  housekeeping.mif

housekeeping.elf: $(PHK)/main.c $(PHK)/w1-hw.c

clean::
	rm -f $(POW)/*.o $(PHK)/main.o $(PHK)/w1-hw.o
