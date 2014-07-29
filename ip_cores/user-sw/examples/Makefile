#BUILD = hello_world
BUILD = one_wire

####################################################################################################

LD              := lm32-elf-ld
CC              := lm32-elf-gcc

TARGET          = user-sw
LOCATION        = ../../../../
MYPATH          = $(LOCATION)/bel_projects/modules/lm32-include
WR              = $(LOCATION)/bel_projects/ip_cores/wrpc-sw
CFLAGS          = -I$(MYPATH)/../ -I$(MYPATH) -mmultiply-enabled -mbarrel-shift-enabled -Os \
                  -W -Wall -Wimplicit -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return \
                  -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wmissing-declarations -Wuninitialized \
                  -I$(WR)/include -I$(WR)/pp_printf

####################################################################################################

ifeq ($(BUILD), hello_world)
SOURCES         = hello_world/main.c
ADD_SOURCES     = $(WR)/dev/uart.c
ADD_FLAGS       =
endif

ifeq ($(BUILD), one_wire)
SOURCES         = one_wire/main.c
ADD_SOURCES     = $(WR)/dev/uart.c $(WR)/dev/w1.c $(WR)/dev/w1-hw.c $(WR)/dev/w1-temp.c $(WR)/dev/w1-eeprom.c 
ADD_FLAGS       = -DBASE_ONEWIRE=pOneWire
endif

####################################################################################################

all: $(TARGET).bin $(TARGET).elf

renew: clean all

read: all
	lm32-elf-readelf -hS $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	lm32-elf-objcopy -O binary $< $@

$(TARGET).elf: $(MYPATH)/aux.c  $(MYPATH)/ebm.c $(MYPATH)/timer.c $(MYPATH)/display.c $(MYPATH)/irq.c \
	$(MYPATH)/mini_sdb.c $(MYPATH)/mprintf.c $(WR)/arch/lm32/crt0.S \
	$(SOURCES) $(ADD_SOURCES)
	$(CC) $(CFLAGS) $(ADD_FLAGS) -o $@ -nostdlib -T linker.ld -Wl,--defsym,_fstack=131072-4 -lgcc -lc $^ 

clean:
	rm -f *.o *.elf *.bin

test: all
	lm32-elf-readelf -hS $(TARGET).elf
	lm32-ctl load $(TARGET).elf

