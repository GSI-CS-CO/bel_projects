CC      := lm32-elf-gcc
CFLAGS  := -Wall -mmultiply-enabled -mbarrel-shift-enabled -Os -ggdb -ffreestanding

dispdemo.bin: dispdemo.elf
	lm32-elf-objcopy -O binary $< $@

dispdemo.elf: crt0.o display.o main.o
	$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^


clean:
	rm -f *.o *.elf *.bin
