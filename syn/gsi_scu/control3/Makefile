TARGET      = scu_control
DEVICE      = EP2AGX125
FLASH       = EPCS128
RAM_SIZE    = 180224
SHARED_SIZE = 80K
USRCPUCLK   = 125000
PATHSCU     = ../../../top/gsi_scu
CFLAGS      = -I. -I$(PSCU)/include -I$(WRPC_IMPORT) -I$(PATHSCU) -DHISTORY -fno-toplevel-reorder

include ../../build.mk

CFLAGS        += -O2

$(TARGET).elf: $(PATHSCU)/main.c  $(INCPATH)/display.c  $(PATHSCU)/cb.c \
               $(PATHSCU)/scu_bus.c $(PATHSCU)/scu_mil.c $(PATHSCU)/fg.c  \
               $(PATHSCU)/dow_crc.c $(PATHSCU)/history.c \
               $(INCPATH)/mprintf.c $(INCPATH)/irq.c $(INCPATH)/mini_sdb.c \
               $(INCPATH)/stack-check.c $(INCPATH)/aux.c $(INCPATH)/sdb_add.c \
               $(INCPATH)/assert.c \
               $(WRPC_IMPORT)/printf.c $(WRPC_IMPORT)/uart.c $(WRPC_IMPORT)/timer.c \
               $(WRPC_IMPORT)/vsprintf-full.c $(WRPC_IMPORT)/div64.c

clean::
	rm -f $(PATHSCU)/main.o $(W1)/dev/*.o
