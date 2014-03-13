TARGET		= scu_diob
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PDIOB     = ../../top/scu_diob
PDEV      = $(PHK)/dev
W1        = ../../ip_cores/wrpc-sw
STUBD       = ../../modules/lm32_stub
GENRAMMIF   = ../../ip_cores/wrpc-sw/tools/genrammif
STUBS       = $(STUBD)/stubs.c $(STUBD)/crt0.S
CFLAGS      = -I$(PHK)/include -I$(PDIOB) -mmultiply-enabled \
              -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf

include ../build.mk

$(TARGET).mif:  housekeeping.mif

housekeeping.elf: $(PHK)/main.c $(PDEV)/mprintf.c $(W1)/dev/uart.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PDEV)/*.o $(POW)/*.o $(PHK)/main.o $(W1)/dev/*.o
