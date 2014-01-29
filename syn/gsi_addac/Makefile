TARGET		= scu_addac
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 16384
PATHADDAC   = ../../top/gsi_addac
GENRAMMIF = ../../ip_cores/wrpc-sw/tools/genrammif
STUBD   = ../../modules/lm32_stub
STUBS   = $(STUBD)/stubs.c $(STUBD)/crt0.S


include ../build.mk

scu_addac.elf: $(PATHADDAC)/main.c
