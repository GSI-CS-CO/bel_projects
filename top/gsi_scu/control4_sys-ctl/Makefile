#Makefile for MCU based SCU4 power control
#Kai Lüghausen <k.lueghausen@gsi.de> 

CC          := avr-gcc 
CC_FLAGS    := -Os
CC_TARGET	:= -mmcu=atxmega64a1
LIBS        :=
SPEED		:= -DF_CPU=32000000UL
DEBUG       := -g -Wall
FILE        := control4_sys-ctl


.PHONY: cleanobj comp debug

SRCS=$(wildcard *.c)

OBJ =$(SRCS:.c=.o)



%.o : %.c
	$(CC) $(CC_TARGET) $(CC_FLAGS) $(SPEED) -c $(CC_FLAGS) $< -o $@

all: comp

debug: CC_TARGET += -g
debug: comp

comp: $(OBJ)
	$(CC) $(CC_TARGET) $(OBJ) -o $(FILE).elf
	avr-objcopy -O ihex -j .text -j .data $(FILE).elf $(FILE).hex

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.hex








install:
	avrdude -c atmelice_pdi -p x64a1 -U flash:w:$(FILE).hex
