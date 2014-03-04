TARGET		= scu_control
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 131072
PATHSCU   = ../../../top/gsi_scu/control3
PSCU      = ../../../modules/scu_bus/housekeeping_sw
PDEV      = $(PSCU)/dev
W1        = ../../../ip_cores/wrpc-sw
CFLAGS    = -I$(PSCU)/include -I$(PSCU)/include/hw -I$(PATHSCU)/../ -mmultiply-enabled \
            -mbarrel-shift-enabled -Os -I$(W1)/include

include ../../build.mk

$(TARGET).elf: $(PATHSCU)/../main.c $(PATHSCU)/../aux.c $(PATHSCU)/../display.c $(PATHSCU)/../irq.c $(PATHSCU)/../scu_bus.c $(PATHSCU)/../mini_sdb.c \
  $(PDEV)/mprintf.c $(W1)/dev/uart.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PDEV)/*.o $(PATHSCU)/../main.o $(W1)/dev/*.o

                
