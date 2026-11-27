TARGET		= blm_aco
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PBLM     = ../../top/blm_aco
W1        = ../../ip_cores/wrpc-sw
CFLAGS    = -I$(PHK)/include -I$(PBLM)
USRCPUCLK = 125000

include ../build.mk

$(TARGET).mif:  housekeeping.mif

# housekeeping uses its own w1-hw.c (SDB-discovered BASE_ONEWIRE), w1 core and temperature from wrpc-import
housekeeping.elf: $(PHK)/main.c $(PHK)/w1-hw.c $(WRPC_IMPORT)/dev/w1.c $(WRPC_IMPORT)/dev/w1-temp.c

clean::
	rm -f $(POW)/*.o $(PHK)/main.o $(PHK)/w1-hw.o
