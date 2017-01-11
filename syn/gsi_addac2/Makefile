TARGET		= scu_addac2
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PADDAC    = ../../top/gsi_addac2
W1        = ../../ip_cores/wrpc-sw
CFLAGS    = -I$(PHK)/include -I$(PADDAC)
USRCPUCLK = 125000
PATHPKG   = $(shell python2.7 ../../ip_cores/hdl-make/hdlmake list-mods | grep -G '^[^\#]' | grep top | grep -o '^\S*')

include ../build.mk

$(TARGET).sof:  housekeeping.mif

housekeeping.elf: $(PHK)/main.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(POW)/*.o $(PHK)/main.o $(W1)/dev/*.o
