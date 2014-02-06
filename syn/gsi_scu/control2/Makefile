TARGET		= scu_control
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 131072
PATHSCU   = ../../../top/gsi_scu/control2

include ../../build.mk

scu_control.elf: $(PATHSCU)/main.c $(PATHSCU)/aux.c $(PATHSCU)/display.c $(PATHSCU)/irq.c $(PATHSCU)/scu_bus.c $(PATHSCU)/mini_sdb.c
