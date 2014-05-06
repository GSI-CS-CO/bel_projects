TARGET		= scu_addac2
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PADDAC    = ../../top/gsi_addac2
PDEV      = $(PHK)/dev
W1        = ../../ip_cores/wrpc-sw
CFLAGS      = -I$(PHK)/include -I$(PADDAC) -mmultiply-enabled \
              -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf

include ../build.mk

$(TARGET).sof:  housekeeping.mif

housekeeping.elf: $(PHK)/main.c $(PDEV)/mprintf.c $(W1)/dev/uart.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PDEV)/*.o $(POW)/*.o $(PHK)/main.o $(W1)/dev/*.o
