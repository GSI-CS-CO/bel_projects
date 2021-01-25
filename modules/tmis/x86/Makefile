# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
#       on asl, don't forget to set the PKG_CONFIG_PATH
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../fw
#DIM         ?=/home/bel/dbeck/lnx/dim_v20r29
DIM         ?=~/schrott/dim_v20r29
INCLUDE     ?=../include
TARGETS     := tmis-server tmis-client

#quick hack fix for buildingn on ASL
ASLLIB      := /common/export/timing-rte/tg-fallout-v6.0.1/x86_64/lib
ASLINC      := /common/export/timing-rte/tg-fallout-v6.0.1/x86_64/include/saftlib

EXTRA_FLAGS ?=
CFLAGS      ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW) -I $(ASLINC) -I../../common-libs/include -I$(DIM)/dim
LIBS        ?= `pkg-config  saftlib --libs` -L . -L $(ASLLIB) -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib $(DIM)/linux/libdim.a -lm -letherbone -lpthread



all: $(TARGETS)

tmis-server: tmis-server.cpp
	g++ --std=c++0x $(CFLAGS) -o tmis-server tmis-server.cpp $(LIBS)

tmis-client: tmis-client.cpp
	g++ --std=c++0x $(CFLAGS) -o tmis-client tmis-client.cpp $(LIBS)

clean:
	rm -f *.o tmis-server tmis-client

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	

.PHONY: all clean
