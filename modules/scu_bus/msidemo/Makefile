LD      := lm32-elf-ld
CC      := lm32-elf-gcc
CFLAGS  := -Wall -mmultiply-enabled -mbarrel-shift-enabled -Os -ggdb -DCPU_CLOCK=62500

msidemo.bin: msidemo.elf
	lm32-elf-objcopy -O binary $< $@

msidemo.elf: crt0.o mini_sdb.o aux.o display.o irq.o scu_bus.o main.o
	$(CC) $(CFLAGS) -o $@ -T ram.ld $^


clean:
	rm -f *.o *.elf *.bin
