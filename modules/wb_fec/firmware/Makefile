CROSS_COMPILE ?= lm32-elf-
export CROSS_COMPILE

CC 			= $(CROSS_COMPILE)gcc
LD 			= $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE 		= $(CROSS_COMPILE)size

CONFIG_PRINT_BUFSIZE ?= 256
CONFIG_RAMSIZE = 180224

WRPC     := ../../../ip_cores/wrpc-sw
ARCH		 := ./arch/lm32
HW			 := ./hw
PRINT 	 := ./printf
SDB			 := ./sdb

OUTPUT   := fec_dec

#SRCS     := $(ARCH)/irq.c $(SDB)/mini_sdb.c $(WRPC)/dev/uart.c $(SDB)/sdb_arg.c main_missing.c\

SRCS     := $(ARCH)/irq.c $(SDB)/mini_sdb.c $(WRPC)/dev/uart.c $(SDB)/sdb_arg.c main_missing.c\
						$(WRPC)/pp_printf/printf.c $(WRPC)/pp_printf/vsprintf-full.c deco.c
ASS_SRCS := $(ARCH)/sdb_add.S $(ARCH)/crt0.S

OBJ      := $(SRCS:.c=.o) 
OBJ      += $(ASS_SRCS:.S=.o)
CLEAN    :=	*.o *.elf *.bin *.mif
CFLAGS   := -Wall -mmultiply-enabled -mbarrel-shift-enabled -Os 
CFLAGS   += -I$(WRPC)/pp_printf/ -I./ -I$(ARCH) -I$(SDB) -I$(PRINT) -I$(HW) -I$(WRPC)/include
CFLAGS   += -DCONFIG_PRINT_BUFSIZE=$(CONFIG_PRINT_BUFSIZE)

all: $(OUTPUT).elf $(OUTPUT).mif

$(OUTPUT).elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ -nostdlib -T $(ARCH)/linker.ld $^
	$(SIZE) $@

%.mif: %.elf
	../../../ip_cores/wrpc-sw/tools/genrammif $*.elf $(CONFIG_RAMSIZE) > $@

clean:
	rm -f $(HW)/$(CLEAN) $(ARCH)/$(CLEAN) $(PRINT)/$(CLEAN) $(SDB)/$(CLEAN) $(CLEAN)

