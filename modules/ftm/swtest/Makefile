LD      := lm32-elf-ld
CC      := lm32-elf-gcc
#CFLAGS  := -I$(W1)/include -I$(W1)/pp_printf -Wall -mmultiply-enabled -mbarrel-shift-enabled -Os -ggdb -ffreestanding
TARGET    = ftm
MYPATH    = .
W1        = ../../../ip_cores/wrpc-sw
CFLAGS    = -I$(MYPATH)/../ -mmultiply-enabled -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf




$(TARGET).bin: $(TARGET).elf
	lm32-elf-objcopy -O binary $< $@

$(TARGET).elf:  $(MYPATH)/aux.c  $(MYPATH)/ebm.c $(MYPATH)/timer.c $(MYPATH)/display.c $(MYPATH)/irq.c \
                $(MYPATH)/mini_sdb.c $(MYPATH)/main.c $(MYPATH)/crt0.o $(MYPATH)/mprintf.c \
                $(W1)/dev/uart.c 
#$(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c
	$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^ 


clean:
	rm -f *.o *.elf *.bin
