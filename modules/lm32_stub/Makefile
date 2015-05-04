# Makefile Stub
# M. Kreider 28.01.2015

TARGET     = myproject
VERSION    = "0.1.0"

MYPATH     = .
CHECKOUT   = $(shell git rev-parse --show-toplevel)
#CHECKOUT   = $(shell ls Makefile)

MYSRC      = $(MYPATH)/main.c
RAMSIZE    = 64K # must be less or equal to synthesized RAM size 
STACKSIZE  = 8K  # size of the stack, 8K recommended
SHAREDSIZE = 8K  # size of fixed memory range for data exchange with the host system

MYSYM      = -DPASS_MY_VAR=42
MYFLAGS    = $(MYSYM)

all:
	@echo $(CHECKOUT)

include $(CHECKOUT)/modules/lm32-include/build_lm32.mk
