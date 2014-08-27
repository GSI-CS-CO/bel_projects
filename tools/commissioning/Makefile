# Makefile for device test files
# - Will be used at activation.sh
# - Example: make device-test-vetar2a
# ====================================================================================================
PREFIX      ?= /usr/local
EB          ?= ../../ip_cores/etherbone-core/api
EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB)
LIBS        ?= -L $(EB) -Wl,-rpath,$(PREFIX)/lib -letherbone

%:	%.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

%:	%.cpp
	g++ $(CFLAGS) -o $@ $< $(LIBS) -leca -ltlu

