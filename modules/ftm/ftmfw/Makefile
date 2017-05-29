TARGET			= ftm
DEBUGLVL  	= 0
FLASH				= EPCQ256
RAM_SIZE		= 49152 
SHARED_SIZE = 8192
USRCPUCLK   = 125000
VERSION     = 0.0.6

include ../../../syn/build.mk

PATHFW 	= .
PATHPRIO = ../../prioq2
PREFIX 	?= /usr/local
CFLAGS 	+= -I$(PATHFW) -I$(PATHPRIO) -DDEBUGLEVEL=$(DEBUGLVL) 

all: $(TARGET).bin $(TARGET).elf

$(TARGET).elf:	$(PATHFW)/main.c $(INCPATH)/ebm.c
clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin
