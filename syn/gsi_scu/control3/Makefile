TARGET      = scu_control
DEVICE      = EP2AGX125
FLASH       = EPCS128
RAM_SIZE    = 147456
SHARED_SIZE = 80K
USRCPUCLK   = 125000
PATHSCU     = ../../../top/gsi_scu
CFLAGS      = -I. -I$(PSCU)/include -I$(PATHSCU) -DHISTORY -fno-toplevel-reorder

include ../../build.mk

CFLAGS        += -O2

doc:
	doxygen Doxyfile
$(TARGET).elf: $(PATHSCU)/main.c  $(INCPATH)/display.c  $(PATHSCU)/cb.c \
               $(PATHSCU)/scu_bus.c $(PATHSCU)/scu_mil.c $(PATHSCU)/fg.c  \
               $(PATHSCU)/dow_crc.c $(PATHSCU)/history.c

clean::
	rm -f $(PATHSCU)/main.o $(W1)/dev/*.o
