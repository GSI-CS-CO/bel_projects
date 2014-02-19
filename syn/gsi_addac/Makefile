TARGET		= scu_addac
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PADDAC   = ../../modules/scu_bus/housekeeping_sw
PDEV    = $(PADDAC)/dev
POW     = $(PDEV)/sockitowm
STUBD       = ../../modules/lm32_stub
GENRAMMIF   = ../../ip_cores/wrpc-sw/tools/genrammif
STUBS       = $(STUBD)/stubs.c $(STUBD)/crt0.S
CFLAGS      = -I$(PADDAC)/include -I$(PADDAC)/include/hw -I$(PADDAC)/include/sockitowm -mmultiply-enabled \
              -mbarrel-shift-enabled -Os

include ../build.mk

scu_addac.sof:  housekeeping.mif

housekeeping.elf: $(PADDAC)/main.o $(PDEV)/mprintf.o $(PDEV)/uart.o $(POW)/owerr.o $(POW)/owlnk.o $(POW)/ownet.o \
              $(POW)/owtran.o $(POW)/eep43.o $(POW)/crcutil.o $(POW)/temp42.o $(POW)/temp10.o $(POW)/temp28.o \
              $(POW)/findtype.o

clean::
	rm -f $(PDEV)/*.o $(POW)/*.o $(PADDAC)/main.o
