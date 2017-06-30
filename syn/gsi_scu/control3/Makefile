TARGET		  = scu_control
DEVICE		  = EP2AGX125
FLASH		    = EPCS128
RAM_SIZE	  = 96000
HARED_SIZE = 24K
USRCPUCLK   = 125000

PATHSCU   = ../../../top/gsi_scu
CFLAGS    = -I$(PSCU)/include -I$(PATHSCU)

include ../../build.mk

$(TARGET).elf: $(PATHSCU)/main.c  $(INCPATH)/display.c  $(PATHSCU)/cb.c \
                $(PATHSCU)/scu_bus.c $(PATHSCU)/scu_mil.c $(PATHSCU)/fg.c  \
                $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PATHSCU)/main.o $(W1)/dev/*.o

                
