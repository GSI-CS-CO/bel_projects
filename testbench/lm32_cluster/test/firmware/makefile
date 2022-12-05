RAM_SIZE=131072

all: firmware.mif 

LM32_FEATURE_FLAGS = 		\
	-mbarrel-shift-enabled	\
	-mmultiply-enabled		\
	-msign-extend-enabled	

firmware.mif: firmware.bin genrammif
	./genrammif $< $(RAM_SIZE) > $@

firmware.bin: firmware
	lm32-elf-objcopy -O binary firmware firmware.bin

firmware: crt0.S firmware.c irq.c aux.c mini_sdb.c sdb_add.c mprintf.c printf.c
	lm32-elf-gcc $(LM32_FEATURE_FLAGS) -std=gnu99 -Wl,-Map=firmware.map -T ram.ld -O4 -o firmware crt0.S firmware.c irq.c aux.c mini_sdb.c sdb_add.c mprintf.c -I.


genrammif: genrammif.c
	gcc -o $@ $<
