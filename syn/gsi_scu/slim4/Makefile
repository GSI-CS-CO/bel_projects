TARGET      = scu4slim
DEVICE      = 10AX027E3
FLASH       = MT25QU512
SPI_LANES   = ASx4
RAM_SIZE    = 262144
SHARED_SIZE = 80K
USRCPUCLK   = 125000
PATHSCU     = ../../../top/gsi_scu
CFLAGS      = -I. -I$(PSCU)/include -I$(PATHSCU) -DHISTORY -fno-toplevel-reorder -DSCU4SLIM

include ../../build.mk

CFLAGS        += -O2

doc:
	doxygen Doxyfile
$(TARGET).elf: $(PATHSCU)/main.c  $(INCPATH)/display.c  $(PATHSCU)/cb.c \
                $(PATHSCU)/scu_bus.c $(PATHSCU)/scu_mil.c $(PATHSCU)/fg.c  \
		$(PATHSCU)/dow_crc.c $(PATHSCU)/history.c \
                $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PATHSCU)/main.o $(W1)/dev/*.o
