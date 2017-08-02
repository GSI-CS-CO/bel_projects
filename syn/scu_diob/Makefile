TARGET		= scu_diob
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PDIOB     = ../../top/scu_diob
W1        = ../../ip_cores/wrpc-sw
CFLAGS    = -I$(PHK)/include -I$(PDIOB)
USRCPUCLK = 125000

include ../build.mk

$(TARGET).mif:  housekeeping.mif

housekeeping.elf: $(PHK)/main.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(POW)/*.o $(PHK)/main.o $(W1)/dev/*.o
