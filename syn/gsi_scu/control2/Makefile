TARGET		= scu_control
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 131072
PATHSCU   = ../../../top/gsi_scu
LM32INC   = ../../../modules/lm32-include
PSCU      = ../../../modules/scu_bus/housekeeping_sw
PDEV      = $(PSCU)/dev
W1        = ../../../ip_cores/wrpc-sw
CFLAGS    = -I$(PSCU)/include -I$(PATHSCU) -I$(LM32INC) -mmultiply-enabled \
            -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf

include ../../build.mk

#$(TARGET).elf: $(PATHSCU)/main.c $(LM32INC)/aux.c $(LM32INC)/display.c $(LM32INC)/irq.c $(PATHSCU)/cb.c \
#                $(PATHSCU)/scu_bus.c $(LM32INC)/mini_sdb.c $(PATHSCU)/fg.c $(LM32INC)/mprintf.c $(W1)/dev/uart.c \
#                $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

clean::
	rm -f $(PDEV)/*.o $(PATHSCU)/main.o $(W1)/dev/*.o

                
