TARGET		= example
DEVICE		= 5AGXMA3D4
FLASH		= EPCQ256
SPI_LANES	= ASx4
RAM_SIZE	= 49152 
SHARED_SIZE     = 8K
USRCPUCLK       = 125000
VERSION         = 0.0.1

PATHFW    = .
PATHPKG		= $(shell python2.7 ../../ip_cores/hdl-make/hdlmake list-mods | grep -G '^[^\#]' | grep top | grep -o '^\S*')
PREFIX    ?= /usr/local
CFLAGS    = -I$(PATHFW) 
include   ../../syn/build.mk

all: $(TARGET).elf 

$(TARGET).elf: $(PATHFW)/main.c $(INCPATH)/ebm.c 

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin

