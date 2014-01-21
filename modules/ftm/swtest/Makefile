LD      := lm32-elf-ld
CC      := lm32-elf-gcc
CFLAGS  := -Wall -mmultiply-enabled -mbarrel-shift-enabled -Os -ggdb -ffreestanding

msidemo.bin: msidemo.elf
	lm32-elf-objcopy -O binary $< $@

msidemo.elf:  ftm.o aux.o ebm.o timer.o display.o irq.o mini_sdb.o main.o crt0.o
	$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^ 


clean:
	rm -f *.o *.elf *.bin
