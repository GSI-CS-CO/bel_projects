# Makefile for ssd1325 display demo
# * $@ is the file name of the target
# * $< is the name of the first dependency
# ====================================================================================================

LD							:= lm32-elf-ld
CC							:= lm32-elf-gcc

TARGET					= ssd1325_display_demo
LOCATION				= ../../../../..
SSD1325_FILES		= $(LOCATION)/bel_projects/modules/ssd1325/src/c
MYPATH					= $(LOCATION)/bel_projects/modules/lm32-include
W1							= $(LOCATION)/bel_projects/ip_cores/wrpc-sw
CFLAGS					= -I$(MYPATH)/../ -I$(MYPATH) -mmultiply-enabled -mbarrel-shift-enabled -Os \
								  -W -Wall -Wimplicit -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return \
								  -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wmissing-declarations -Wuninitialized \
								  -I$(W1)/include -I$(W1)/pp_printf -I$(SSD1325_FILES)

all:						$(TARGET).bin $(TARGET).elf

read:						all
								lm32-elf-readelf -hS $(TARGET).elf

$(TARGET).bin:	$(TARGET).elf
								lm32-elf-objcopy -O binary $< $@

$(TARGET).elf:  $(MYPATH)/aux.c  $(MYPATH)/ebm.c $(MYPATH)/timer.c $(MYPATH)/display.c $(MYPATH)/irq.c \
								$(MYPATH)/mini_sdb.c $(MYPATH)/mprintf.c \
								$(W1)/dev/uart.c $(W1)/arch/lm32/crt0.S \
								$(SSD1325_FILES)/ssd1325_serial_driver.c \
								main.c
								$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld -Wl,--defsym,_fstack=131072-4 -lgcc -lc $^ 
		
test:						all
								lm32-elf-readelf -hS $(TARGET).elf
								lm32-ctl load $(TARGET).elf
		
uart:						test
								sudo minicom -D /dev/ttyACM0
			
clean:
								rm -f *.o *.elf *.bin
