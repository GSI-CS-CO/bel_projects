# PREFIX  controls where programs and libraries get installed
# Example usage:
#   make PREFIX=/usr all
@usage: make all
PWD     := $(shell pwd)
HOME    = $(PWD)/..
ODIR    = $(HOME)/eb_memtest
IDIR    = $(HOME)/eb_memtest
WBDIR   = $(HOME)/wb_api
DEVDIR  = $(HOME)/wb_devices
PREFIX  ?= /usr/local
SVNDIR  = $(HOME)/../production/utility/i386
INCLUDE = -I$(IDIR) -I$(WBDIR) -I$(DEVDIR)
LIBS	= -letherbone
FLAG    = DONTSIMULATE

all: eb_memtest

eb_memtest: $(IDIR)/eb_memtest.c
	@echo Making eb_memtest
	gcc -g -Wall -D$(FLAG) $(INCLUDE) -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_memtest $(IDIR)/eb_memtest.c  $(LIBS)

clean:
	@echo cleaning stuff
	rm -f eb_memtest


