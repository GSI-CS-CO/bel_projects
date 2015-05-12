TARGET		= ftm
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 131072
SHARED_SIZE     = 8K
USRCPUCLK       = 125000

PATHFTM   = ../../../modules/ftm/ftmfw
CFLAGS    = -I$(PATHFTM)

include ../../build.mk

$(TARGET).elf: $(PATHFTM)/main.c $(PATHFTM)/ftm.c $(INCPATH)/ebm.c 

clean::
	rm -f $(PATHFTM)/*.o


