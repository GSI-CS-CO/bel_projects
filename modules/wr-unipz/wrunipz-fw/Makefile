TARGET		= wrunipz
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 131072
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 00.00.01

.DEFAULT_GOAL  := fwbin

PATHFW 	        = .
DEBUGLVL    	= 1
CFLAGS 	    	= -I$(PATHFW) -DDEBUGLEVEL=$(DEBUGLVL)

include ../../../syn/build.mk

fwbin: $(TARGET).bin 

$(TARGET).elf: $(PATHFW)/wr-unipz.c $(INCPATH)/ebm.c $(PATHFW)/../../../top/gsi_scu/scu_mil.c

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin


