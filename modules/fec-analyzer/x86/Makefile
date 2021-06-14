# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../fw
INCLUDE     ?=../include
TARGETS     := fec-analyzer

#quick hack fix for buildingn on ASL

ASLLIB      := /common/export/timing-rte/tg-fallout-v6.0.1/x86_64/lib
ASLINC      := /common/export/timing-rte/tg-fallout-v6.0.1/x86_64/include/saftlib

EXTRA_FLAGS ?=
#CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW) -I../../common-libs/include
#LIBS        ?= -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib
CFLAGS      ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW) -I $(ASLINC)
LIBS        ?= `pkg-config  saftlib --libs` -L . -L $(ASLLIB) -L $(EB)/.libs -L $(DIMPATH)/linux -Wl,-rpath,$(PREFIX)/lib -lm -letherbone 



all: $(TARGETS)

fec-analyzer: fec-analyzer.cpp
	g++ --std=c++0x $(CFLAGS) -o fec-analyzer fec-analyzer.cpp $(LIBS)

clean:
	rm -f *.o fec-analyzer

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	

.PHONY: all clean
