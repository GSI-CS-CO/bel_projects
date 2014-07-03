LD      := lm32-elf-ld
CC      := lm32-elf-gcc
TARGET    = verifyftm
MYPATH    = .
FTMPATH   = ../swtest
INCPATH   = ../../lm32-include
W1        = ../../../ip_cores/wrpc-sw
CFLAGS    = -I$(INCPATH) -mmultiply-enabled -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf

$(TARGET).bin: $(TARGET).elf
	lm32-elf-objcopy -O binary $< $@

$(TARGET).elf: $(INCPATH)/aux.c $(INCPATH)/irq.c $(FTMPATH)/ftm.c \
               $(INCPATH)/mini_sdb.c $(MYPATH)/main.c $(INCPATH)/crt0.o \
               $(INCPATH)/mprintf.c $(W1)/dev/uart.c 
		         $(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^ 

clean:
	rm -f *.o *.elf *.bin
