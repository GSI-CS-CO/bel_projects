TARGET		= scu_diob
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PDIOB     = ../../modules/scu_bus/housekeeping_sw
PDEV    = $(PDIOB)/dev
POW     = $(PDEV)/sockitowm
STUBD       = ../../modules/lm32_stub
GENRAMMIF   = ../../ip_cores/wrpc-sw/tools/genrammif
STUBS       = $(STUBD)/stubs.c $(STUBD)/crt0.S
CFLAGS      = -I$(PDIOB)/include -I$(PDIOB)/include/hw -I$(PDIOB)/include/sockitowm -mmultiply-enabled \
              -mbarrel-shift-enabled -Os

include ../build.mk

$(TARGET).mif:  housekeeping.mif

housekeeping.elf: $(PDIOB)/main.o $(PDEV)/mprintf.o $(PDEV)/uart.o $(POW)/owerr.o $(POW)/owlnk.o $(POW)/ownet.o \
              $(POW)/owtran.o $(POW)/eep43.o $(POW)/crcutil.o $(POW)/temp42.o $(POW)/temp10.o $(POW)/temp28.o \
              $(POW)/findtype.o

clean::
	rm -f $(PDEV)/*.o $(POW)/*.o $(PDIOB)/main.o
