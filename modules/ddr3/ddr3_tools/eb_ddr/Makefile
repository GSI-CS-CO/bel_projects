# PREFIX  controls where programs and libraries get installed
# Example usage:
#   make PREFIX=/usr all
@usage: make all
PWD     := $(shell pwd)
HOME    = $(PWD)/..
ODIR    = $(HOME)/eb_ddr
IDIR    = $(HOME)/eb_ddr
WBDIR   = $(HOME)/wb_api
DEVDIR  = $(HOME)/wb_devices
PREFIX  ?= /usr/local
SVNDIR  = $(HOME)/../production/utility/i386
INCLUDE = -I$(IDIR) -I$(WBDIR) -I$(DEVDIR)
LIBS	= -letherbone
FLAG    = DONTSIMULATE

all: eb_ddr

eb_ddr: $(IDIR)/eb_ddr.c
	@echo Making eb_ddr
	gcc -g -Wall -D$(FLAG) $(INCLUDE) -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_ddr $(IDIR)/eb_ddr.c  $(LIBS)

clean:
	@echo cleaning stuff
	rm -f eb_ddr


